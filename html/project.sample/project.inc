<?php

// Sample BOINC project web config file.
// Modify it to suit your project.
// Put your version in html/project/project.inc
//
// add tra() around visible strings to make them translatable
// see inc/translation.inc and
// https://boinc.berkeley.edu/trac/wiki/TranslateProject for details

use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\SMTP;
use PHPMailer\PHPMailer\Exception;

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/PHPMailer/autoload.php");

//-------------- Project name and owner

define("PROJECT", "Asteroids@home");
define("COPYRIGHT_HOLDER", " Asteroids@home | Astronomical Institute st Charles University of Prague");

//-------------- URLs and directories

$master_url = parse_config(get_config(), "<master_url>");
define("URL_BASE", $master_url);
$base = parse_config(get_config(), "<base>");
define("BASE", $base);
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");

//-------------- contact info

define("SYS_ADMIN_EMAIL", "admin@$master_url");
define("UOTD_ADMIN_EMAIL", "admin@$master_url");
    // who gets emails about user of the day pool running low?
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");
    // Email addresses separated by pipe ( | ) that will receive user reports
    // of offensive forum posts.

// ------------ PHPMailer configuration

$cfg_file = get_config();
$useSMTPAuth = parse_boolint($cfg_file,  "<useSMTPAuth>");
$SMTPSecure = parse_config($cfg_file , "<SMTPSecure>");
$mail_host = parse_config($cfg_file, "<mail_host>" );
$mail_port = parse_config($cfg_file, "<mail_port>");
$mail_username = parse_config($cfg_file, "<mail_username>");
$mail_password = parse_config($cfg_file, "<mail_password>");

$mail_from = parse_config($cfg_file, "<mail_from>");
$mail_add_reply_to_email = parse_config($cfg_file, "<mail_add_reply_to_email>");
$mail_add_reply_to_name = parse_config($cfg_file, "<mail_add_reply_to_name>");
$mail_add_set_from_email = parse_config($cfg_file, "<mail_add_set_from_email>");
$mail_add_set_from_name = parse_config($cfg_file, "<mail_add_set_from_name>");

//-------------- Delete Account
//define("DELETE_DELAY", 2);
    // When deleting an account, invalidate the authenticator and then wait this
    // many seconds before proceeding with the delete.  This is intended to give the
    // an existing scheduler request sufficient time to complete.  Some projects
    // might want to increase this to a longer time.  Simply uncomment and set the
    // delay to what the project needs.
    
//-------------- Caching

//define("MEMCACHE_SERVERS", "127.0.0.1:11211");

//-------------- CSS styling

// add your own stylesheets (included after bootstrap)
//define("STYLESHEET", "aaa.css");
//define("STYLESHEET2", "bbb.css");
define("STYLESHEET", "/css/mainlight.css");
define("STYLESHEET2", "/css/customlight.css");
define("STYLESHEET3", "/css/font-awesome.min.css");

//-------------- enable/disable web features

define("FORUM_QA_MERGED_MODE", true);
    // Set to true to merge Message boards and Q&A section
define ("DISABLE_PROFILES", false);
    // enable profiles only after enabling reCAPTCHA
    // https://boinc.berkeley.edu/trac/wiki/ProtectionFromSpam
define("USE_STOPFORUMSPAM", true);
    // use http://www.stopforumspam.com to suppress spammer accounts
define("RPC_DEBUG", false);
    // Enables PHP error messages in public Web RPCs
define("TERMSOFUSE_FILE", "../../terms_of_use.txt");
    // Defines the location of the terms of use file. Default location is in the project directory.

//-------------- Project-specific preferences

define('COLOR_PREFS', false);
    // user can select screensaver color scheme
define('GFX_CPU_PREFS', false);
    // user can limit % CPU used by screensaver
    // (lower frame rate if exceeded)
    // This is probably irrelevant if your screensaver uses OpenGL
define('APP_SELECT_PREFS', false);
    // user can choose which apps to run
define('NON_GRAPHICAL_PREF', false);
    // user can choose to run faster non-graphical app versions if available
define('MAX_JOBS_PREF', false);
    // user can choose max # of jobs in progress
define('MAX_CPUS_PREF', false);
    // user can choose max # of CPUs to use

global $boincstats_id;
$boincstats_id = 134;

class BoincCustomUser extends BoincUser{
    static function sum($field, $clause=null) {
        $db = BoincDb::get();
        return $db->sum('user', $field);
    }

    static function count($clause="TRUE") {
        $db = BoincDb::get();
        return $db->count('user');
    }
}

class BoincCustomWorkunit extends BoincWorkunit{
    static function count($clause="TRUE") {
        $db = BoincDb::get();
        return $db->count('workunit', $clause);
    }
}

function parse_boolint($xml, $tag) {
    $x = parse_bool($xml, $tag);
    if ($x) return 1;
    return 0;
}

//-------------- PHPMailer

// If you use PHPMailer, uncomment the following
// and complete the function definition based on your SMTP server
// (not all fields may be needed)
//
//if (0) {
function make_php_mailer() {
    // $mail = new PHPMailer();
    // $mail->IsSMTP();
    // //$mail->Mailer = 'smtp';
    // $mail->SMTPAuth = true;
    // $mail->SMTPSecure = "tls";
    // $mail->Host = "smtp.gmail.com";
    // $mail->Port = 587;
    // $mail->Username = "john.doe@gmail.com";
    // $mail->Password = "xxx";
    //     // Google's application-specific password,
    //     // if you are using the 2-Step Verification: 16 characters, no spaces.
    //     // OR: Put here the regular Gmail password if you are not using the
    //     // 2-Step Verification with your Gmail account.
    //     // See https://support.google.com/accounts/answer/185833?hl=en";
    // $mail->SetFrom('admin@boincproject.com', 'John Doe');
    // $mail->AddReplyTo("admin@boincproject.com", "John Doe");
    // $mail->From = "admin@boincproject.com";

    global $useSMTPAuth, $SMTPSecure, $mail_host, $mail_port, $mail_username, $mail_password, $mail_add_set_from_email, $mail_add_set_from_name,
        $mail_add_reply_to_email, $mail_add_reply_to_name, $mail_from;
    
    date_default_timezone_set('Etc/UTC');

    $mail = new PHPMailer();
    $mail->IsSMTP();
    $mail->SMTPDebug = SMTP::DEBUG_SERVER;
    $mail->SMTPAuth = $useSMTPAuth;
    $mail->SMTPSecure = $SMTPSecure;
    $mail->Host = $mail_host;
    $mail->Port = $mail_port;
    $mail->Username = $mail_username;
    $mail->Password = $mail_password;
        // Google's application-specific password,
        // if you are using the 2-Step Verification: 16 characters, no spaces.
        // OR: Put here the regular Gmail password if you are not using the
        // 2-Step Verification with your Gmail account.
        // See https://support.google.com/accounts/answer/185833?hl=en";
    $mail->SetFrom($mail_add_set_from_email, $mail_add_set_from_name);
    $mail->AddReplyTo($mail_add_reply_to_email, $mail_add_reply_to_name);
    $mail->From = $mail_from;
    //$mail->SMTPDebug  = 3;
    //$mail->Debugoutput = function($str, $level) {echo "debug level $level; message: $str";}; //$mail->Debugoutput = 'echo';

    // $mail->DebugOutput = function ($str, $level) {
    //     file_put_contents(
    //       '../../phpmailer.log',
    //       date('Y-m-d H:i:s') . "\t" . $str,
    //       FILE_APPEND | LOCK_EX
    //     );
    //   };

    return $mail;
}

//}

//-------------- Header and footer

// Put your project title and logo here
// If you include any links, prepend URL with $prefix
// if you need to open divs here close them in project_footer()
//
function project_banner($title, $prefix, $is_main) {
    echo '
    <header>
    ';

    if ($is_main) {
        // echo '
        // <img class="img-banner" src="img/water.jpg">
        // ';
        //echo ' <img class="img-fluid" style="width:100%" src="img/water.jpg">';

        echo '        
        <div class="row banner-row">
            <!--<img src="img/asteroids_at_home_banner.png" class="img-banner"/>-->
            <img src="img/asteroids_at_home_logo.png" class="overlay-trilogo" />
            <img src="img/white_grad.png" class="banner_overlay"/>
            <!--<object type="image/svg+xml" data="img/logo_mff_en.svg" alt="Trio-Logo" class="overlay-trilogo">Trio-Logo</object>
            <img src="img/253px-Charles-University-symbol-4.svg.png" alt="CUNI" class="overlay-cuni-stacked"/>
            <a id="link_asteroidsathome" class="hide-ext-link" href="http://astro.troja.mff.cuni.cz/index_en.html" title="Faculty of Mathematics and Physics" target="_blank"></a>
            <a id="link_cuni" href="http://www.cuni.cz/UKENG-1.html" title="Charles University"></a>
            <a id="link_cuni2" href="http://www.cuni.cz/UKENG-1.html">Charles University</a>
            -->
        ';
        echo '    
        </div>

    ';
    }

    // echo '    
    // <div class="row blue">
    // ';
    sample_navbar(secure_url_base(), get_logged_in_user(false), false);
    echo '         
    </header>
    <div class="container-fluid">
        <div class="row flex-xl-nowrap justify-content-md-center">
        <main class="col-md-9 col-xl-8 py-md-3 pl-md-5 bd-content" role="main">
    ';

    if($title){
        echo ' 
        <div class="row">
        <div class="col-lg-12">
        <!--<div class="mainnav">-->
        <h2 class="headline">'.$title.'</h2>
        <!--</div>-->
        </div>
        </div>    
        
        ';
    }
}

function project_footer($show_return, $show_date, $prefix) {
    global $base;
    // If you include any links, prepend URL with $prefix
    //
    echo '
        </main>
        </div>
        </div>

        <footer class="footer">
        <div class="container">
        <div class="row">
        <div class="col-md-4">
            <!-- COLUMN 1 -->
            <img src="'.$prefix.'img/asteroids_at_home_logo.png" title="Asteroids@home"
                alt="Asteroids at home" class="logo">
            <h3 class="sr-only">ABOUT US</h3>
            <p>
                The first Czech project in the system of distributed computing BOINC and the first in the Czech
                Republic in the field of astronomy. This is Asteroids@home, a project that the Czech National
                Team has helped bring to the world.
            </p>
            <!-- <p>Proactively aggregate B2B initiatives before extensive channels. Monotonectally extend interactive methods of empowerment through excellent applications. Rapidiously synergize visionary products with sticky technology.</p> -->
            <br>
            <address class="margin-bottom-30px">
                <ul class="list-unstyled address">
                    <li>Faculty of Mathematics and Physics
                        <br>Charles University
                    </li>
                    <li>Ke Karlovu 3</li>
                    <li>121 16 Praha 2</li>
                    <li>Czech Republic</li>
                </ul>
            </address>
            <!-- END COLUMN 1 -->
        </div>
        <div class="col-md-4">
            <!-- COLUMN 2 -->
            <h3 class="footer-heading">USEFUL LINKS</h3>
            <div class="row margin-bottom-30px">
                <div class="col-xs-6">
                    <ul class="list-unstyled footer-nav">
                        <li><a href="'.$prefix.'../cs/article01.html">'.tra("About %1", PROJECT).'</a></li>
                        <li><a href="'.$prefix.'old_news.php"/>'.tra("News").'</a></li>
                        <li><a href="'.$prefix.'forum_index.php">'.tra("Message boards").'</a></li>
                        <li><a href="'.$prefix.'team.php">'.tra("Tems").'</a></li>
                        <li><a href="'.$prefix.'welcome.php">'.tra("Help").'</a></li>
                    </ul>
                </div>
                <div class="col-xs-6">
                    <ul class="list-unstyled footer-nav">
                        <li><a href="../files/poster_DPS_2012.pdf">Press Kit</a></li>
                        <li><a href="'.$prefix.'forum_help_desk.php">'.tra("FAQ").'</a></li>
                        <li><a href="'.$prefix.'info.php">Terms & Conditions</a></li>
                        <li><a href="http://www.mff.cuni.cz/toUTF8.en/fakulta/struktura/lide/2968.htm">Contact Us</a></li>
                    </ul>
                </div>
            </div>
            <!-- END COLUMN 2 -->
        </div>
        <div class="col-md-4">
            <!-- COLUMN 3 -->
            <div class="row">
                <div class="col-md-12 newsletter">
                    <h3 class="footer-heading">NEWSLETTER</h3>
                    <p>Get the latest update from us by subscribing to our newsletter.</p>
                    <form class="newsletter-form" method="POST">
                        <div class="input-group input-group-lg">
                            <input type="email" class="form-control" name="email"
                                placeholder="youremail@domain.com">
                            <span class="input-group-btn"><button class="btn btn-primary" type="button"><i
                                        class="fa fa-spinner fa-spin"></i><span>SUBSCRIBE</span></button>
                            </span>
                        </div>
                        <div class="alert"></div>
                    </form>
                </div>
            </div>
            <div class="row justify-content-around">
                <div class="column-md-6 social-connect">
                    <h3 class="footer-heading">GET CONNECTED</h3>
                    <ul class="list-inline social-icons">
                        <li><a href="https://twitter.com/AsteroidsAtHome" class="twitter-bg" title="twitter"><i class="fa fa-twitter"></i></a></li>
                        <li><a href="https://www.facebook.com/profile.php?id=100086279088328" class="facebook-bg" title="facebook"><i class="fa fa-facebook"></i></a></li>
                        <li><a href="'.$prefix.'rss_main.php" class="rss-bg" title="rss"><i class="fa fa-rss"></i></a></li>
                    </ul>
                </div>
                <div class="column-md-6 social-connect">
                    <h3 class="footer-heading">POWERED BY</h3>
                    <ul class="list-inline social-icons">
                        <li>
                            <a href="http://boinc.berkeley.edu/"><img class="rounded d-block" style="height: 50px;"
                                src="'.$prefix.'img/boinc-logo.png" alt="Powered by BOINC"></a>
                        </li>
                    </ul>
                </div>
            </div>
            
            <!-- END COLUMN 3 -->
        </div>
    </div>
        </div>
        <!-- COPYRIGHT -->
        <div class="row text-center copyright">
            <div class="col-md-12">
            <span>Copyright &copy; '.gmdate("Y ").COPYRIGHT_HOLDER.'</span>
            </div>
        </div>
        <!-- END COPYRIGHT -->
    ';
    // echo '<br>
    //     <div class="container">
    //     <a class="brand boinc-logo" href="https://boinc.berkeley.edu/"><img class="img-responsive center-block" src="'.secure_url_base().'img/pb_boinc.gif" alt="Powered by BOINC"></a>
    //     <div class="form-group"></div>
    //     <p class="text-center"> &copy;'.gmdate("Y ").COPYRIGHT_HOLDER.'</p>
    // ';
    if ($show_date) {
        $t = time_str(time());
        echo "<center><small>".tra("Generated")." $t</small><center>\n";
    }
    echo '</footer>
    ';
}

// function project_footer($show_return, $show_date, $prefix) {
//     // If you include any links, prepend URL with $prefix
//     //
//     echo '<br>
//         <a class="brand boinc-logo" href="https://boinc.berkeley.edu/"><img class="img-responsive center-block" src="'.secure_url_base().'img/pb_boinc.gif" alt="Powered by BOINC"></a>
//         <div class="form-group"></div>
//         <p class="text-center"> &copy;'.gmdate("Y ").COPYRIGHT_HOLDER.'</p>
//     ';
//     if ($show_date) {
//         $t = time_str(time());
//         echo "<center><small>".tra("Generated")." $t</small><center>\n";
//     }
// }

//-------------- Ops access control

// Authorize access to administrative pages.
// You can check for login, IP address, or whatever you want.
//
function auth_ops_example() {
    // if running from cmdline, skip checks
    //
    if (!$_SERVER['REMOTE_ADDR']) {
        return;
    }

    // example: require login as a particular user (id 1 in this case)
    //
    if (0) {
        auth_ops_userid(array(1));
        return;
    }

    // example: require that logged-in user have ADMIN or DEV flags
    // set in their forum_prefs.privilege
    //
    if (0) {
        auth_ops_privilege();
        return;
    }
}

//-------------- Customizable functions

function show_profile_heading1() {
    return tra("Your personal background.");
}

function show_profile_question1() {
    return tra("Tell us about yourself. You could tell us where you're from, your age, occupation, hobbies, or anything else about yourself.");
}

function show_profile_heading2() {
    return tra("Your opinions about %1", PROJECT);
}

function show_profile_question2() {
    return tra("Tell us your thoughts about %1<ol>
    <li>Why do you run %1?
    <li>What are your views about the project?
    <li>Any suggestions?
    </ol>", PROJECT);
}

function project_workunit($wu){
    // shown in the workunit page
}

function project_user_summary($user){
    // shown in the user summary page
}

function project_user_page_private($user){
    // shown in the private account page
}

function project_forum_post_rules() {
    // additional forum post rules
    return "";
}

function project_delete_account($user) {
    // a project defined function for deleting users if neither
    // obfuscate_account($user) or wipe_account($user) in 
    // inc/delete_account.inc meets the need of the project
    die("This function must be implemented before it can be used");
}

//-------------- Support for per-app credit

// if (0) {

// show project-specific credit on user/team pages
//
function show_app_credit_user($user, $app_name, $appids) {
    $t = 0;
    $a = 0;
    $n = 0;
    foreach ($appids as $appid) {
        $cu = BoincCreditUser::lookup(
            "userid=$user->id and appid=$appid and credit_type=0"
        );
        if ($cu) {
            $t += $cu->total;
            $a += $cu->expavg;
            $n += $cu->njobs;
        }
    }
    row2("$app_name credit",
        format_credit_large($t)." total, ".
        format_credit($a)." average".
        " ($n tasks)"
    );
}

function show_app_credit_team($team, $app_name, $appids) {
    $t = 0;
    $a = 0;
    $n = 0;
    foreach ($appids as $appid) {
        $ct = BoincCreditTeam::lookup(
            "teamid=$team->id and appid=$appid and credit_type=0"
        );
        if ($ct) {
            $t += $ct->total;
            $a += $ct->expavg;
            $n += $ct->njobs;
        }
    }
    row2("$app_name credit",
        format_credit_large($t)." total, ".
        format_credit($a)." average".
        " ($n tasks)"
    );
}

function show_app_totals($app_name, $appids) {
    $t = 0;
    $a = 0;
    $n = 0;
    foreach ($appids as $appid) {
        $cu = BoincCreditUser::lookup(
            "appid=$appid and credit_type=0"
        );
        if ($cu) {
            $t += $cu->total;
            $a += $cu->expavg;
            $n += $cu->njobs;
        }
    }
    row2("$app_name credit",
        format_credit_large($t)." total, ".
        format_credit($a)." average".
        " ($n tasks)"
    );
}

// a list of "sub-projects", used in the display of per-app credit and badges.
// A subproject is:
// - a set of 1 or more apps; an app can belong to at most 1 subproject.
// - a name, shown on the web site
// - a short name, used in badge names.  Don't use "total".
//
$sub_projects = array(
    array("name" => "Remote Test", "short_name" => "RT", "appids" => array(16)),
    array("name" => "Uppercase", "short_name" => "UC", "appids" => array(1, 25)),
    array("name" => "AsteroidsAthome", "short_name" => "UC", "appids" => array(3)),
);

function project_user_credit($user){
    global $sub_projects;

    foreach ($sub_projects as $sp) {
        show_app_credit_user($user, $sp["name"], $sp["appids"]);
    }
}

function project_team_credit($team) {
    global $sub_projects;
    foreach ($sub_projects as $sp) {
        show_app_credit_team($team, $sp["name"], $sp["appids"]);
    }
}

// if (!defined('STATUS_PAGE_TTL')) {
//     define('STATUS_PAGE_TTL', 3600);
// }

function get_sched_daemon(){
    $c = simplexml_load_file("../../config.xml");
    if (!$c) {
        die("can't parse config file\n");
    }

    $daemons = $c->daemons;
    $config = $c->config;
    $main_host = trim((string)$config->host);
    $local_daemons = array();

    if ($config->sched_host) {
        $sched_host = trim((string) $config->sched_host);
    } else {
        $sched_host = $main_host;
    }
    
    if ($sched_host == $main_host) {
        $y = new StdClass;
        $y->cmd = "Scheduler";
        $y->host = $sched_host;
        $y->status = !file_exists("../../stop_sched");;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }

    $x = new StdClass;
    $x->local_daemons = $local_daemons;

    return $x;
}

function get_daemon_status_for_summary($d) {
    switch ($d->status) {
    case 0:
        $s = tra("Not Running");
        // $c = "bg-danger";
        break;
    case 1:
        $s = tra("Running");
        // $c = "bg-success";
        break;
    default:
        $s = tra("Disabled");
        // $c = "bg-warning";
        break;
    }

    return $s;
}

function get_job_status_for_summary() {
    $s = unserialize(get_cached_data(STATUS_PAGE_TTL, "job_status"));
    if ($s) {
        return $s;
    }

    $s = new StdClass;
    
    $s->users_total = BoincUser::count("1=1");
    $s->users_with_recent_credit = BoincUser::count("expavg_credit>1");
    $s->users_with_credit = BoincUser::count("total_credit>1");
    $s->users_past_24_hours = BoincUser::count("create_time > (unix_timestamp() - 86400)");
    $s->hosts_with_recent_credit = BoincHost::count("expavg_credit>1");
    $s->hosts_with_credit = BoincHost::count("total_credit>1");
    $s->hosts_past_24_hours = BoincHost::count("create_time > (unix_timestamp() - 86400)");
    $s->credit_past_24_hours = BoincCustomUser::sum("total_credit", "create_time > (unix_timestamp() - 86400)");
    $s->total_credits = BoincUser::sum("total_credit");
    $s->flops = BoincUser::sum("expavg_credit")/200;
    $s->results_ready_to_send = BoincResult::count("server_state=2");
    $s->results_in_progress = BoincResult::count("server_state=4");
    $s->results_in_progress_new = BoincResult::count("server_state=4");
    $s->wus_total = BoincCustomWorkunit::count();
    $s->wus_valid_past_24_hours = BoincResult::count("server_state=5 and outcome=1 and validate_state=2 and received_time > (unix_timestamp() - 86400)");
    
    return $s;
}

function project_app_credit(){
    echo '[project_totals()]
    ';
    global $sub_projects;
    foreach ($sub_projects as $sp) {
        show_app_totals($sp["name"], $sp["appids"]);
    }
}

function admin_index_extra(){
    echo 'Radim, do you need more functionality on this page? G.V.
    ';
    
    }

// }   // if(0)



?>
