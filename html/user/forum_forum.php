<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// display the threads in a forum.

require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');
require_once('../inc/pm.inc');

check_get_args(array("id", "sort", "start"));

$id = get_int("id");
$sort_style = get_int("sort", true);
$start = get_int("start", true);
if (!$start) $start = 0;

$forum = BoincForum::lookup_id($id);
if (!$forum) error_page("forum ID not found");
$user = get_logged_in_user(false);
BoincForumPrefs::lookup($user);

if (DISABLE_FORUMS && !is_admin($user)) {
    error_page("Forums are disabled");
}

if (!is_forum_visible_to_user($forum, $user)) {
    if ($user) {
        remove_subscriptions_forum($user->id, $id);
    }
    error_page(tra("Not visible to you"));
}

if (!$sort_style) {
    // get the sort style either from the logged in user or a cookie
    if ($user){
        $sort_style = $user->prefs->forum_sorting;
    } else {
        list($sort_style, $thread_style) = parse_forum_cookie();
    }
} else {
    // set the sort style
    if ($user){
        $user->prefs->forum_sorting = $sort_style;
        $user->prefs->update("forum_sorting=$sort_style");
    } else {
        list($old_style, $thread_style) = parse_forum_cookie();
        send_cookie(
            'sorting', implode("|", array($sort_style, $thread_style)), true
        );
    }
}


switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category);
    if ($category->is_helpdesk) {
        page_head(tra("Questions and Answers").' : '.$forum->title);
    } else {
        //page_head(tra("Message boards").' : '.$forum->title);
        page_head($forum->title);
    }

    //test_table();
    show_forum_header($user);
    show_forum_title($category, $forum, NULL);
    break;

case 1:
    $team = BoincTeam::lookup_id($forum->category);
    page_head(tra("Team message board for %1", $team->name));

    show_forum_header($user);
    show_team_forum_title($forum);
    break;
}

// TITLE END

function render_create_thread_button($id, $user, $forum){
    echo '
        <div class="col-auto  mr-auto">
        <form action="forum_forum.php" method="get" class="form-inline">
        <input type="hidden" name="id" value="'.$forum->id.'">
        ';
    
        if (user_can_create_thread($user, $forum)) {
            show_button(
                "forum_post.php?id=$id", tra("New thread"), tra("Add a new thread to this forum")
            );
        }
    
        echo '</form>
        </div>
        ';
}

function render_rss_message($forum){
    echo '<div class="row">
    <div class="col-sm-12">
        '.
        tra("This message board is available as an %1 RSS feed %2", "<a href=forum_rss.php?forumid=$forum->id&setup=1>", "<img src=img/feed_logo.png></a>");
    
        // echo'</main>
        // </div>
        // ';
        echo '</div>
        </div>
        </div>
        ';
}

echo '<div class="row">
';
render_create_thread_button($id, $user, $forum);
render_sort_post_form($forum_sort_styles, $sort_style, "forum_forum.php", $forum->id);
echo '    </div>
';

//test_table();
show_forum($forum, $start, $sort_style, $user);
render_rss_message($forum);
page_tail();


// This function shows the threads for the given forum
// Starting from $start,
// using the given $sort_style (as defined in forum.php)
// and using the features for the logged in user in $user.
//
function show_forum($forum, $start, $sort_style, $user) {
    // return;
    $page_nav = page_links(
        "forum_forum.php?id=$forum->id&amp;sort=$sort_style",
        $forum->threads,
        THREADS_PER_PAGE,
        $start
    );
    echo $page_nav;
    start_table('table-striped');
    row_heading_array(
        array(
            "",
            tra("Threads"),
            tra("Posts"),
            tra("Author"),
            tra("Views"),
            "<nobr>".tra("Last post")."</nobr>"
        ),
        array("", "width=35%", "", "", "", "")

    );

    echo '<tbody>
    ';

    $sticky_first = !$user || !$user->prefs->ignore_sticky_posts;

    // Show hidden threads if logged in user is a moderator
    //
    $show_hidden = is_moderator($user, $forum);
    $threads = get_forum_threads(
        $forum->id, $start, THREADS_PER_PAGE,
        $sort_style, $show_hidden, $sticky_first
    );

    if ($user) {
        $subs = BoincSubscription::enum("userid=$user->id");
    }

    // Run through the list of threads, displaying each of them
    //
    foreach ($threads as $thread) {
        $owner = BoincUser::lookup_id($thread->owner);
        if (!$owner) continue;
        $unread = thread_is_unread($user, $thread);

        //if ($thread->status==1){
            // This is an answered helpdesk thread
        if ($user && is_subscribed($thread, $subs)) {
            echo '<tr class="bg-info">';
        } else {
            // Just a standard thread.
            echo '<tr>';
        }

        // echo "<td width=\"1%\"><nobr>";
        $alt = "Sticky";
        $src = "img/sticky_post.png";
        echo "<td><nobr>";
        if ($thread->hidden) {
            show_image(IMAGE_HIDDEN, tra("This thread is hidden"), tra("hidden"));
        } else if ($unread) {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_STICKY_LOCKED, tra("This thread is sticky and locked, and you haven't read it yet"), tra("sticky/locked/unread"));
                } else {
                    show_image(NEW_IMAGE_STICKY, tra("This thread is sticky and you haven't read it yet"), tra("sticky/unread"));
                }
            } else {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_LOCKED, tra("You haven't read this thread yet, and it's locked"), tra("unread/locked"));
                } else {
                    show_image(NEW_IMAGE, tra("You haven't read this thread yet"), tra("unread"));
                }
            }
        } else {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(IMAGE_STICKY_LOCKED, tra("This thread is sticky and locked"), tra("sticky/locked"));
                } else {
                    show_image(IMAGE_STICKY, tra("This thread is sticky"), tra("sticky"));
                } //echo "<img class=\"icon\" border=\"0\" title=\"$thread->title\" alt=\"$alt\" src=\"$src\" $h>";
            } else {
                if ($thread->locked) {
                    show_image(IMAGE_LOCKED, tra("This thread is locked"), tra("locked"));
                } else {
                    show_image(IMAGE_POST, tra("You read this thread"), tra("read"));
                }
            }
        }
        echo "</nobr></td>";

        $title = cleanup_title($thread->title);
        //$titlelength = 9999;
        //if (strlen($title) > $titlelength) {
        //    $title = substr($title, 0, $titlelength)."...";
        //}
        echo "<td><a href=\"forum_thread.php?id=$thread->id\">$title</a><br></td>";

        echo '
            <td>'.($thread->replies+1).'</td>
            <td>'.user_links($owner, BADGE_HEIGHT_SMALL).'</td>
            <td>'.$thread->views.'</td>
            <td>'.time_diff_str($thread->timestamp, time()).'</td>
            </tr>
        ';
        flush();
    }
    echo '</tbody>
    ';
    end_table();
    echo "$page_nav";    // show page links
}

?>
