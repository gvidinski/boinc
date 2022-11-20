<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// This is a template for your web site's front page.
// You are encouraged to customize this file,
// and to create a graphical identity for your web site.
// by customizing the header/footer functions in html/project/project.inc
// and picking a Bootstrap theme
//
// If you add text, put it in tra() to make it translatable.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/text_transform.inc");
// require_once("../project/project.inc");
require_once("../inc/bootstrap.inc");

$config = get_config();
$no_web_account_creation = parse_bool($config, "no_web_account_creation");
$project_id = parse_config($config, "<project_id>");
    
$stopped = web_stopped();
$user = get_logged_in_user(false);

// The panel at the top of the page
//
function panel_contents() {
}

function top() {
    global $stopped, $master_url, $user;
    if ($stopped) {
        echo '
            <p class="lead text-center">'
            .tra("%1 is temporarily shut down for maintenance.", PROJECT)
            .'</p>
        ';
    }
    //panel(null, 'panel_contents');
}

function left(){
    global $user, $no_web_account_creation, $master_url, $project_id;
    $title = $user?tra("Welcome, %1", $user->name):tra("What is %1?", PROJECT);
    panel($title,
        function() use($user, $title) {
            global $no_web_account_creation, $master_url, $project_id;
            if ($user) {
                $dt = time() - $user->create_time;
                if ($dt < 86400) {
                    echo tra("Thanks for joining %1", PROJECT);
                } else if ($user->total_credit == 0) {
                    echo tra("Your computer hasn't completed any tasks yet.  If you need help, %1go here%2.",
                            "<a href=https://boinc.berkeley.edu/help.php>",
                            "</a>"
                    );
                } else {
                    $x = format_credit($user->expavg_credit);
                    echo tra("You've contributed about %1 credits per day to %2 recently.", $x, PROJECT);
                    if ($user->expavg_credit > 1) {
                        echo " ";
                        echo tra("Thanks!");
                    } else {
                        echo "<p><p>";
                        echo tra("Please make sure BOINC is installed and enabled on your computer.");
                    }
                }
                echo "<p><p>";
                echo sprintf('<center><a href=home.php class="btn btn-success">%s</a></center>
                    ',
                    tra('Continue to your home page')
                );
                echo "<p><p>";
                echo sprintf('%s
                    <ul>
                    <li> %s
                    <li> %s
                    <li> %s
                    ',
                    tra("Want to help more?"),
                    tra("If BOINC is not installed on this computer, %1download it%2.",
                        "<a href=download_software.php>", "</a>"
                    ),
                    tra("Install BOINC on your other computers, tablets, and phones."),
                    tra("Tell your friends about BOINC, and show them how to join %1.", PROJECT)
                );
                if (function_exists('project_help_more')) {
                    project_help_more();
                }
                echo "</ul>\n";
            } else {
                echo '              <div class="mainnav">
                                <h2 class="headline">'.$title.'</h2>
                ';
                $pd = "../project/project_description.php";
                if (file_exists($pd)) {
                    include($pd);
                } else {
                    echo "No project description yet. Create a file html/project/project_description.php
                        that prints a short description of your project.
                    ";
                }

                if (NO_COMPUTING) {
                    if (!$no_web_account_creation) {
                        echo "
                            <a href=\"create_account_form.php\">Create an account</a>
                        ";
                    }
                } else {
                    // use auto-attach if possible
                    //
                    if (!$no_web_account_creation) {
                        echo '  <center>
                        <a href="signup.php" class="btn btn-success"><font size=+2>'.tra('Join %1', PROJECT).'</font></a>
                        </center>
                    ';
                    }
                    echo '<p><p>'.tra("Already joined? %1Log in%2.",
                        "<a href=login_form.php>", "</a>").'
                        </div>
                        ';
                }
            }

            // echo '<hr class="my-4">
            echo '<br>
            ';

            global $stopped;
            if (!$stopped) {
                $profile = get_current_uotd();
                if ($profile) {
                    //panel(tra('User of the Day'),
                    //function() use ($profile) {
                        show_uotd($profile);
                    //}
                    //);
                }
            }
            //require_once("../project/server_summary.inc");
            //get_server_summary();
        }
    );
}

function right() {
    panel(tra('News'),
        function() {
            include("motd.php");
            if (!web_stopped()) {
                echo '      <h2>'.tra('News').'</h2>
                ';
                show_news(0, 5);
                echo '
                ';
            }
            
            
        }
    );

    // echo '</div>
    //     <hr class="my-4">
    // ';
    
}

function links_panel($url_prefix){
    echo '<div class="row">
    <div class="col-lg-6">
    <div class="card mb-6">
        <h3 class="card-header">More info</h3>
        <div class="card-body">
            <ul>
                <li> <a href="http://asteroidsathome.net/">'.tra("Detailed information about the project (English and Czech)").'</a></li>
                <li> <a target="_blank" href="http://astro.troja.mff.cuni.cz/projects/asteroids3D/">DAMIT - Database of Asteroid Models from Inversion Techniques (English)</a></li>
                <li> <a target="_blank" href="http://www.rni.helsinki.fi/~mjk/asteroids.html">Articles and math (English)</a></li>
                <li> <a target="_blank" href="http://asteroidsathome.net/cs/article01.html">Detailed article about project (Czech)</a></li>
            </ul>
        </div>
    </div>
</div>
<div class="col-lg-6">
<div class="card mb-6">
    <h3 class="card-header">Participate</h3>
    <div class="card-body">
        <ul>

            <li><a href="'.$url_prefix.'info.php">'.tra("Read our rules and policies").'</a>
            </li>
            <li>1) <a href="http://boinc.berkeley.edu/download.php">'.tra("Download").'</a>, <strong>'.tra("install").'</strong> '.tra("and").' <strong>'.tra("run").'</strong> '.tra("the BOINC software").';<br>
            
                2) '.tra("When prompted, enter the URL: ").' <strong>'.$url_prefix.'</strong>
            </li>
            <li> '.tra("If you have any problems").',
                <a href="http://boinc.berkeley.edu/wiki/BOINC_Help" target="_blank">'.tra("get help here").'</a>


            </li>
        </ul>
    </div>
</div>
</div>
</div>
<br>
<div class="row">
<div class="col-lg-6">
    <div class="card mb-6">
        <h3 class="card-header">Returning participants</h3>
        <div class="card-body">
            <ul>
                <li><a href="'.$url_prefix.'home.php">Your account</a> - view stats, modify
                    preferences
                </li>
                <li><a href="'.$url_prefix.'server_status.php">Server status</a>
                </li>
                <li><a href="'.$url_prefix.'cert1.php">Certificate</a>
                </li>
                <li><a href="'.$url_prefix.'apps.php">Applications</a>
                </li>
                <li><a href="'.$url_prefix.'team.php">Teams</a> - create or join a team
                </li>
            </ul>
            <br>
        </div>
    </div>
</div>
<div class="col-lg-6">
    <div class="card mb-6">
        <h3 class="card-header">Community</h3>
        <div class="card-body">
            <ul>
                <li><a href="'.$url_prefix.'profile_menu.php">Profiles</a>
                </li>
                <li><a href="'.$url_prefix.'user_search.php">User search</a>
                </li>
                <li><a href="'.$url_prefix.'forum_index.php">Message boards</a>
                </li>
                <li><a href="'.$url_prefix.'forum_help_desk.php">Questions and Answers</a>
                </li>
                <li><a href="'.$url_prefix.'stats.php">Statistics</a>
                </li>
                <li><a href="'.$url_prefix.'language_select.php">Languages</a>
                </li>
            </ul>
        </div>
    </div>
</div>
</div>


    ';
}

page_head(null, null, true);
// echo '<div class="row">
// ';
grid('top', 'left', 'right');

// echo '</div>
// ';

// $lp = "../project/link_panes.php";
// if (file_exists($lp)) {
// include($lp);

links_panel(secure_url_base(),);

page_tail(false, "", true);

?>
