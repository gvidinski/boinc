<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Get server status.
//
// default: show as web page
// ?xml=1:  show as XML
// ?counts=1: show only overall job counts, w/o caching
//      (for remote job submission systems)
// Sources of data:
// - daemons on this host: use "ps" to see if each is running
//   (this could be made more efficient using a single "ps",
//   or it could be cached)
// - daemons on other hosts: get from a cached file generated periodically
//   by ops/remote_server_status.php
//   (we can't do this ourselves because apache can't generally ssh)
// - apps and job counts: get from a cached file that we generate ourselves

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/server_version.inc");
// require_once("../project/project.inc");
require_once("../project/daemons.inc");

if (!defined('STATUS_PAGE_TTL')) {
    define('STATUS_PAGE_TTL', 3600);
}

function daemon_xml($d) {
    switch ($d->status) {
    case 0: $s = "not running"; break;
    case 1: $s = "running"; break;
    default: $s = "disabled";
    }
    echo "  <daemon>
        <host>$d->host</host>
        <command>".command_display($d->cmd)."</command>
        <status>$s</status>
    </daemon>
";
}

function item_xml($name, $val) {
    if (!$val) $val = 0;
    echo "   <$name>$val</$name>\n";
}

function item_html($name, $val) {
    $name = tra($name);
    echo "<tr><td>$name</td><td>$val</td></tr>\n";
    //echo "<tr><td align=right>$name</td><td align=right>$val</td></tr>\n";
}

function show_hosts(){
    if(file_exists("../project/hosts.html")){
        echo '<h3>'.tra("Hosts").'</h3>
        ';
        
        start_table("table-condensed");
        include("../project/hosts.html");
        end_table();
    }
}

function show_status_html($x) {
    global $server_version, $server_version_str;
    page_head(tra("Project status"));
    $j = $x->jobs;
    $daemons = $x->daemons;
    start_table();
    echo "<tr><td>\n";
            echo "
                 <h3>".tra("Server status")."</h3>
            ";
            start_table('table-striped');
            table_header(tra("Program"), tra("Host"), tra("Status"));
            foreach ($daemons->local_daemons as $d) {
                daemon_html($d);
            }
            foreach ($daemons->remote_daemons as $d) {
                daemon_html($d);
            }
            foreach ($daemons->disabled_daemons as $d) {
                daemon_html($d);
            }
            end_table();

            if ($daemons->cached_time) {
                echo "<br>Remote daemon status as of ", time_str($daemons->cached_time);
            }
            if ($daemons->missing_remote_status) {
                echo "<br>Status of remote daemons is missing\n";
            }
            if (function_exists('server_status_project_info')) {
                echo "<br>";
                server_status_project_info();
            }
    echo "</td><td>\n";
            echo "<h3>".tra("Computing status")."</h3>\n";
            echo "<h4>".tra("Work")."</h4>\n";
            start_table('table-striped');
            item_html("Tasks ready to send", $j->results_ready_to_send);
            item_html("Tasks in progress", $j->results_in_progress);
            item_html("Workunits waiting for validation", $j->wus_need_validate);
            item_html("Workunits waiting for assimilation", $j->wus_need_assimilate);
            item_html("Workunits waiting for file deletion", $j->wus_need_file_delete);
            item_html("Tasks waiting for file deletion", $j->results_need_file_delete);
            item_html("Transitioner backlog (hours)", number_format($j->transitioner_backlog, 2));
            end_table();
            echo "<h4>".tra("Users")."</h4>\n";
            start_table('table-striped');
            item_html("With credit", $j->users_with_credit);
            item_html("With recent credit", $j->users_with_recent_credit);
            item_html("Registered in past 24 hours", $j->users_past_24_hours);
            end_table();
            echo "<h4>".tra("Computers")."</h4>\n";
            start_table('table-striped');
            item_html("With credit", $j->hosts_with_credit);
            item_html("With recent credit", $j->hosts_with_recent_credit);
            item_html("Registered in past 24 hours", $j->hosts_past_24_hours);
            item_html("Current TeraFLOPS", round($j->flops / 1000, 2));
            end_table();
    echo "</td></tr>\n";
    end_table();

    show_hosts();

    echo "<h3>".tra("Tasks by application")."</h3>\n";
    start_table('table-striped');
    table_header(
        tra("Application"),
        tra("Unsent"),
        tra("In progress"),
        tra("Runtime of last 100 tasks in hours: average, min, max"),
        tra("Users in last 24 hours")
    );
    foreach ($j->apps as $app) {
        if ($app->info) {
            $avg = empty($app->info->avg) ? 0.0 : $app->info->avg;
            $min = empty($app->info->min) ? 0.0 : $app->info->info;
            $max = empty($app->info->max) ? 0.0 : $app->info->max;
            
            $avg = round($avg, 2);
            $min = round($min, 2);
            $max = round($max, 2);
            $x = $max?"$avg ($min - $max)":"---";
            $u = $app->info->users;
        } else {
            $x = '---';
            $u = '---';
        }
        echo "<tr>
            <td>$app->user_friendly_name</td>
            <td>$app->unsent</td>
            <td>$app->in_progress</td>
            <td>$x</td>
            <td>$u</td>
            </tr>
        ";
    }
    end_table();
    
    // show server software version.
    // If it's a release (minor# is even) link to github branch
    //
    echo "Server software version: $server_version_str";
    if ($server_version[1]%2 == 0) {
        $url = sprintf("%s/%d/%d.%d",
            "https://github.com/BOINC/boinc/tree/server_release",
            $server_version[0],
            $server_version[0],
            $server_version[1]
        );
        echo " <a href=\"$url\">View source on Github</a>.";
    }
    echo "<br>\n";

    if ($j->db_revision) {
        echo tra("Database schema version: "), $j->db_revision;
    }
    echo "<p>Task data as of ".time_str($j->cached_time);
    page_tail();
}

function show_status_xml($x) {
    xml_header();
    echo "<server_status>\n<daemon_status>\n";

    $daemons = $x->daemons;
    foreach ($daemons->local_daemons as $d) {
        daemon_xml($d);
    }
    foreach ($daemons->remote_daemons as $d) {
        daemon_xml($d);
    }
    foreach ($daemons->disabled_daemons as $d) {
        daemon_xml($d);
    }
    echo "</daemon_status>\n<database_file_states>\n";
    $j = $x->jobs;
    item_xml("results_ready_to_send", $j->results_ready_to_send);
    item_xml("results_in_progress", $j->results_in_progress);
    item_xml("workunits_waiting_for_validation", $j->wus_need_validate);
    item_xml("workunits_waiting_for_assimilation", $j->wus_need_assimilate);
    item_xml("workunits_waiting_for_deletion", $j->wus_need_file_delete);
    item_xml("results_waiting_for_deletion", $j->results_need_file_delete);
    item_xml("transitioner_backlog_hours", $j->transitioner_backlog);
    item_xml("users_with_recent_credit", $j->users_with_recent_credit);
    item_xml("users_with_credit", $j->users_with_credit);
    item_xml("users_registered_in_past_24_hours", $j->users_past_24_hours);
    item_xml("hosts_with_recent_credit", $j->hosts_with_recent_credit);
    item_xml("hosts_with_credit", $j->hosts_with_credit);
    item_xml("hosts_registered_in_past_24_hours", $j->hosts_past_24_hours);
    item_xml("current_floating_point_speed", $j->flops);
    echo "<tasks_by_app>\n";
    foreach ($j->apps as $app) {
        echo "<app>\n";
        item_xml("id", $app->id);
        item_xml("name", $app->name);
        item_xml("unsent", $app->unsent);
        item_xml("in_progress", $app->in_progress);
        item_xml("avg_runtime", $app->info->avg);
        item_xml("min_runtime", $app->info->min);
        item_xml("max_runtime", $app->info->max);
        item_xml("users", $app->info->users);
        echo "</app>\n";
    }
    echo "</tasks_by_app>
</database_file_states>
</server_status>
";
}

function show_counts_xml() {
    xml_header();
    echo "<job_counts>\n";
    item_xml('results_ready_to_send', BoincResult::count("server_state=2"));
    item_xml('results_in_progress', BoincResult::count("server_state=4"));
    item_xml('results_need_file_delete', BoincResult::count("file_delete_state=1"));
    item_xml('wus_need_validate', BoincWorkunit::count("need_validate=1"));
    item_xml('wus_need_assimilate', BoincWorkunit::count("assimilate_state=1"));
    item_xml('wus_need_file_delete', BoincWorkunit::count("file_delete_state=1"));
    echo "</job_counts>\n";
}

function main() {
    // $x = new StdClass; 
    //     $x->daemons = get_daemon_status();
    //     $x->jobs = get_job_status();
    // show_status_html($x);

    if (get_int('counts', true)) {
        show_counts_xml();
    } else {
        $x = new StdClass; 
        $x->daemons = get_daemon_status();
        $x->jobs = get_job_status();
        if (get_int('xml', true)) {
            show_status_xml($x);
        } else {
            show_status_html($x);
        }
    }
}

main();
?>
