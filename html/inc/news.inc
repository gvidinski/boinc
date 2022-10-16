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

require_once('../inc/boinc_db.inc');

function news_item($date, $title, $post) {
    $user = BoincUser::lookup_id($post->user);
    // echo "<p>";
    echo '<div class="list-group-item list-group-item-action flex-column align-items-start">
    <div class="d-flex w-100 justify-content-between">
    ';
    if ($title) {
        //echo "<span class=\"lead\">$title</span><br>\n";
        echo '<h5 class="mb-3">'.$title.'</h5>
        ';
    }
    $d = time_str($date);
    echo '    </div>
    ';
    $options = new output_options();
    $options->htmlitems = false;
    if (is_string($post)) {
        $forum_link = false;
        $text = output_transform($post, $options);
    } else {
        $forum_link = true;
        $text = output_transform($post->content, $options);
    }
    echo '
        <p class="mb-1">
        '.$text.'
        </p>
        <small class="text-muted">'.$d.'</small> | 
        <small class="text-muted">'.$user->name.'</small> | 
    ';
    if ($forum_link && !DISABLE_FORUMS) {
        echo '
            <small class="text-muted"><a href="'.url_base().'forum_thread.php?id='.$post->thread.'">'.tra("Discuss").'</a></small>
        ';
    }
    // echo "</span>
    //     </div>
    //     <br clear=all>
    //     <hr>
    // ";
    echo '</div>
    ';
}

function news_forum() {
    if (defined("NEWS_FORUM_NAME")) {
        $forum_name = NEWS_FORUM_NAME;
    } else {
        $forum_name = "News";
    }
    return BoincForum::lookup("parent_type=0 and title = '$forum_name'");
}

function is_news_forum($forum) {
    if (defined("NEWS_FORUM_NAME")) {
        $forum_name = NEWS_FORUM_NAME;
    } else {
        $forum_name = "News";
    }
    return $forum->parent_type == 0 && ($forum->title === $forum_name);
}

function show_news($start, $count) {
    echo '<div class="list-group">
    ';
    
    $forum = news_forum();
    if (!$forum) {
        echo "
            No news forum.  Run html/ops/create_forums.php.
        ";
        return;
    }
    
    if (is_array($start)) {
        echo "You have not changed show_news() to use 0 instead of
            deprecated \$project_news as the first param.";
        return;
    }
    
    $lim = "";
    if ($start) {
        if ($count) {
            $lim = "limit $start, $count";
        } else {
            $lim = "limit $start, 99999999";
        }
    } else if ($count) {
        $lim = "limit $count";
    }
    $threads = BoincThread::enum("forum = $forum->id and hidden=0 order by id desc $lim");

    foreach ($threads as $thread) {
        $posts = BoincPost::enum("thread=$thread->id order by id limit 1");
        $post = $posts[0];
        if (strstr($post->content, $thread->title) == $post->content) {
            $title = null;
        } else {
            $title = $thread->title;
        }
        news_item($thread->create_time, $title, $post);
    }

    if ($count) {
        echo "<a href=".url_base()."old_news.php>... more</a>";
    }
    echo '      <small>
    ';
    printf(tra("News is available as an %sRSS feed%s"),
        "<a href=rss_main.php>",
        " &nbsp; <img src=\"img/rss_icon.gif\" alt=\"RSS\"></a>"
    );

    echo "
        </small>
        </div>
        <br>
    ";
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
