<?PHP
    require_once("../project/project.inc");
    require_once("../project/daemons.inc");
    require_once("../inc/util.inc");
    

    function item_html_for_summary($name, $val, $strong=false) {
        $name = tra($name);
        echo '<tr><td class="col-lg-6 text-right">'.$name.'</td><td class="col-lg-6">'.($strong ? '<strong>' : '').''. $val.''.($strong ? '<strong>' : '').' </td></tr>
        ';
        //echo "<tr><td align=right>$name</td><td align=right>$val</td></tr>\n";
    }

    function get_server_summary(){

        $x = new StdClass; 
        $x->daemons = get_sched_daemon();
        $x->jobs = get_job_status_for_summary();
        $sched_daemon = $x->daemons->local_daemons[0];
        $sched_status = get_daemon_status_for_summary($sched_daemon);
        $j = $x->jobs;
        global $boincstats_id;
    
        echo '<div class="card mb-3">
        <h3 class="card-header">'.tra("Server status").'</h3>
        <div class="card-body">
        <span>'.tra("As of").' '.time_str(time()).' | [Scheduler is '.$sched_status.']</span>
        ';
        start_table('');
        // Total queued jobs
        item_html_for_summary(tra("Total queued jobs:"), $j->results_ready_to_send, true);
        // In progress
        item_html_for_summary(tra("In progress:"), $j->results_in_progress);
        item_html_for_summary(tra("In progress:"), $j->results_in_progress_new);
        item_html_for_summary("Total: ", $j->wus_total);
        //Successes last 24h
        item_html_for_summary(tra("Successes last 24h:"), $j->wus_valid_past_24_hours);
        // Users (last day)
        echo'<tr>
            <td class="col-lg-6 text-right">'.tra("Users").'
                <a href="https://boincstats.com/en/stats/'.$boincstats_id.'/project/detail/user" target="_blank">
                    <img src="img/graph.gif" alt="User statistics" vspace="0" hspace="1" border="0">
                </a>('.tra("last day").'):                
            </td>
            <td class="col-lg-6">'.$j->users_with_credit.' ('.($j->users_past_24_hours > 0 ? '+' : '').$j->users_past_24_hours.')
            </td>
        </tr>
        ';
        // Hosts (last day)
        echo'<tr>
            <td class="col-lg-6 text-right">'.tra("Hosts").'
                <a href="https://boincstats.com/en/stats/'.$boincstats_id.'/project/detail/host" target="_blank">
                    <img src="img/graph.gif" alt="User statistics" vspace="0" hspace="1" border="0">
                </a>('.tra("last day").'):                
            </td>
            <td class="col-lg-6">'.$j->hosts_with_credit.' ('.($j->hosts_past_24_hours > 0 ? '+' : '').$j->hosts_past_24_hours.')
            </td>
        </tr>
        ';
        // Credits last 24h
        echo'<tr>
            <td class="col-lg-6 text-right">'.tra("Credits last 24h").'
                <a href="https://boincstats.com/en/stats/'.$boincstats_id.'/project/detail/credit" target="_blank">
                    <img src="img/graph.gif" alt="User statistics" vspace="0" hspace="1" border="0">
                </a>:                
            </td>
            <td class="col-lg-6">'.$j->credit_past_24_hours.' ('.($j->credit_past_24_hours > 0 ? '+' : '').$j->credit_past_24_hours.')
            </td>
        </tr>
        ';
        item_html_for_summary(tra("Total credits:"), round($j->total_credits, 0));
        item_html_for_summary(tra("Current TeraFLOPS:"), round($j->flops / 1000, 0));

        //project_app_credit();

        end_table();
        echo '</div>
        </div>
        ';
        
        // https://www.boincstats.com/stats/134/project/detail/host
        // 
        // Successes last 24h:	458,893
        // Users  (last day ):	1,381,314 (+8)
        // Hosts  (last day ):	4,512,810 (+57)
        // Credits last 24h :	2,273,944
        // Total credits :	145,016,935,391
        // TeraFLOPS estimate:	22.739
    
        }

?>