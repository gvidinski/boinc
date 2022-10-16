<?PHP


require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");



// trim a daemon command for display.
// For now, remove the cmdline args, but show the app if any
//
function command_display($cmd) {
    $x = explode(" -", $cmd);
    $prog = $x[0];
    $x = strpos($cmd, "-app ");
    if ($x) {
        $y = substr($cmd, $x);
        $y = explode(" ", $y);
        $app = $y[1];
        $prog .= " ($app)";
    }
    return $prog;
}

function daemon_html($d) {
    switch ($d->status) {
    case 0:
        $s = tra("Not Running");
        $c = "bg-danger";
        break;
    case 1:
        $s = tra("Running");
        $c = "bg-success";
        break;
    default:
        $s = tra("Disabled");
        $c = "bg-warning";
        break;
    }
    echo "<tr>
        <td>".command_display($d->cmd)."</td>
        <td>$d->host</td>
        <td class=\"$c\"><nobr>$s</nobr></td>
        </tr>
    ";
}

function local_daemon_running($cmd, $pidname, $host) {
    if (!$pidname) {
        $cmd = trim($cmd);
        $x = explode(" ", $cmd);
        $prog = $x[0];
        $pidname = $prog . '.pid';
    }
    $path = "../../pid_$host/$pidname";
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            $pid = trim($pid);
            $out = Array();
            exec("ps -ww $pid", $out);
            foreach ($out as $y) {
                if (strstr($y, (string)$pid)) return 1;
            }
        }
    }
    return 0;
}

// returns a data structure of the form
// local_daemons: array of
//   cmd, status
// remote_daemons: array of
//   cmd, host, status
// disabled_daemons: array of
//   cmd, host
//
function get_daemon_status() {
    $c = simplexml_load_file("../../config.xml");
    if (!$c) {
        die("can't parse config file\n");
    }
    $daemons = $c->daemons;
    $config = $c->config;
    $main_host = trim((string)$config->host);
    $master_url = trim((string)$config->master_url);
    $u = parse_url($master_url);
    if (!array_key_exists("host", $u)) {
        print_r($u);
        die("can't parse URL $master_url");
    }
    $master_host = $u["host"];
    if ($config->www_host) {
        $web_host = trim((string) $config->www_host);
    } else {
        $web_host = $main_host;
    }
    if ($config->sched_host) {
        $sched_host = trim((string) $config->sched_host);
    } else {
        $sched_host = $main_host;
    }
    $have_remote = false;
    $local_daemons = array();
    $disabled_daemons = array();

    // the upload and download servers are sort of daemons too
    //
    $url = trim((string) $config->download_url);
    $u = parse_url($url);
    $h = $u["host"];
    if ($h == $master_host) {
        $y = new StdClass;
        $y->cmd = "Download server";
        $y->host = $h;
        $y->status = 1;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }
    $url = trim((string) $config->upload_url);
    $u = parse_url($url);
    $h = $u["host"];
    if ($h == $master_host) {
        $y = new StdClass;
        $y->cmd = "Upload server";
        $y->host = $h;
        $y->status = !file_exists("../../stop_upload");;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }

    // Scheduler is a daemon too
    //
    if ($sched_host == $main_host) {
        $y = new StdClass;
        $y->cmd = "Scheduler";
        $y->host = $sched_host;
        $y->status = !file_exists("../../stop_sched");;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }

    foreach ($daemons->daemon as $d) {
        if ((int)$d->disabled != 0) {
            $x = new StdClass;
            $x->cmd = (string)$d->cmd;
            $x->host = (string)$d->host;
            if (!$x->host) $x->host = $main_host;
            $x->status = -1;
            $disabled_daemons[] = $x;
            continue;
        }
        $host = $d->host?(string)$d->host:$main_host;
        if ($host != $web_host) {
            $have_remote = true;
            continue;
        }
        $x = new StdClass;
        $x->cmd = (string)$d->cmd;
        $x->status = local_daemon_running($x->cmd, trim($d->pid_file), $web_host);
        $x->host = $web_host;
        $local_daemons[] = $x;

    }

    $x = new StdClass;
    $x->local_daemons = $local_daemons;
    $x->disabled_daemons = $disabled_daemons;
    $x->missing_remote_status = false;
    $x->cached_time = 0;
    $x->remote_daemons = array();
    if ($have_remote) {
        $f = @file_get_contents("../cache/remote_server_status");
        if ($f) {
            $x->remote_daemons = unserialize($f);
            $x->cached_time = filemtime("../cache/remote_server_status");
        } else {
            $x->missing_remote_status = true;
        }
    }
    return $x;
}

function get_job_status() {
    $s = unserialize(get_cached_data(STATUS_PAGE_TTL, "job_status"));
    if ($s) {
        return $s;
    }

    $now=time();
    $s_day=24*3600;
    $d_ago=$now-$s_day;
    $s_week=7*$s_day;
    $w_ago=$now-$s_week;

    $s = new StdClass();
    
    $apps = BoincApp::enum("deprecated=0");
    foreach ($apps as $app) {
        $info = BoincDB::get()->lookup_fields("result", "stdClass",
            "ceil(avg(elapsed_time)/3600*100)/100 as avg,
            ceil(min(elapsed_time)/3600*100)/100 as min,
            ceil(max(elapsed_time)/3600*100)/100 as max,
            count(distinct userid) as users",
            "appid = $app->id
            AND validate_state=1
            AND received_time > (unix_timestamp()-86400)
            "
        );
        $app->info = $info;
        $app->unsent = BoincResult::count("appid=$app->id and server_state=2");
        $app->in_progress = BoincResult::count("appid=$app->id and server_state=4");
    }
    $s->apps = $apps;
    $s->results_ready_to_send = BoincResult::count("server_state=2");
    $s->results_in_progress = BoincResult::count("server_state=4");
    $s->results_need_file_delete = BoincResult::count("file_delete_state=1");
    $s->wus_need_validate = BoincWorkunit::count("need_validate=1");
    $s->wus_need_assimilate = BoincWorkunit::count("assimilate_state=1");
    $s->wus_need_file_delete = BoincWorkunit::count("file_delete_state=1");
    $s->wus_total = BoincWorkunit::count("TRUE");
    $x = BoincDB::get()->lookup_fields("workunit", "stdClass", "MIN(transition_time) as min", "TRUE");
    $gap = (time() - $x->min)/3600;
    if (($gap < 0) || ($x->min == 0)) {
        $gap = 0;
    }
    $s->transitioner_backlog = $gap;
    $s->users_total = BoincCustomUser::count();
    $s->users_with_recent_credit = BoincUser::count("expavg_credit>1");
    $s->users_with_credit = BoincUser::count("total_credit>1");
    $s->users_past_24_hours = BoincUser::count("create_time > (unix_timestamp() - 86400)");
    $s->hosts_with_recent_credit = BoincHost::count("expavg_credit>1");
    $s->hosts_with_credit = BoincHost::count("total_credit>1");
    $s->hosts_past_24_hours = BoincHost::count("create_time > (unix_timestamp() - 86400)");
    $s->hosts_total = BoincHost::count("TRUE");
    $s->hosts_past_7_days = BoincHost::count("rpc_time>$w_ago");
    $s->flops = BoincUser::sum("expavg_credit")/200;
    

    $s->db_revision = null;
    if (file_exists("../../db_revision")) {
        $s->db_revision = trim(file_get_contents("../../db_revision"));
    }

    $s->cached_time = time();
    $e = set_cached_data(STATUS_PAGE_TTL, serialize($s), "job_status");
    if ($e) echo "set_cached_data(): $e\n";
    return $s;
}

?>