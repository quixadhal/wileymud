#!/usr/bin/perl -w

use strict;
use English qw( −no_match_vars );
use HTML::TreeBuilder;
use JSON qw(encode_json);
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

#my $file_prefix = '/space/stuff/Mirrors/MudMirror-2021-10-16/lpmuds.net';
my $file_prefix = '/space/stuff/Mirrors/MudMirror-2020-05-11/lpmuds.net';

my $profile_data = {};
my $missing_profiles = {};
my $posts_by_id = {};

sub trim {
    my $str = shift;
    return undef unless defined $str;

    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    return $str;
}

sub get_categories {
    my $file = "$file_prefix/forum/index.html";
    die "File $file not found." if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $table = $root->look_down(class => 'table_list') or die "No table_list found in $file.";

    my @categories = $table->look_down(id => qr/category_\d+_boards/);
    die "No categories found in $file." if (scalar @categories) < 1;
    my $category_data = {};
    foreach my $category (@categories) {
        my $category_id = $category->attr('id');
        $category_id =~ /category_(\d+)_boards/;
        $category_id = $1;

        my $parent = $category->look_up(class => 'table_list') or die "Cannot find table list for category $category_id.";
        my $cat_head = $parent->look_down(id => "category_$category_id") or die "Cannot find category_$category_id.";
        my $cat_info = $cat_head->look_down(class => 'catbg') or die "Cannot get category name for $category_id.";
        my $category_name = trim $cat_info->as_text;

        $category_data->{$category_id} = {
            category_id     => $category_id,
            category_name   => $category_name,
        };

        my @boards = $category->look_down(id => qr/board_\d+/);
        die "No boards found in category $category_name." if (scalar @boards) < 1;
        foreach my $board (@boards) {
            my $board_id = $board->attr('id');
            $board_id =~ /board_(\d+)/;
            $board_id = $1;

            my $info = $board->look_down(class => 'info') or die "Cannot get info for board $board_id.";

            my $anchor = $info->look_down(class => 'subject') or die "Cannot get url for board $board_id.";
            my $board_url = $anchor->attr('href');
            my $board_name = trim $anchor->as_text;

            my $p = $info->look_down(_tag => 'p') or die "Cannot get board description for $board_name.";
            my $board_title = trim $p->as_text;

            #  <td class="stats windowbg">
            #    <p>1621 Posts <br /> 271 Topics
            #  </td>
            my $stats = $board->look_down(class => 'stats windowbg') or die "Cannot get stats for $board_name.";
            my $p2 = $stats->look_down(_tag => 'p') or die "Cannot get stats for $board_name.";
            my $counts = $p2->as_text;
            $counts =~ /(\d+)\s+Posts\s+(\d+)\s+Topics/;
            my ($post_count, $topic_count) = ($1, $2);

            #  <td class="lastpost">
            #    <p><strong>Last post</strong> by
            #    <a href="../smf/index8615.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;action=profile;u=312">Adam</a>
            #    <br /> in 
            #    <a href="../smf/index7910.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;topic=1641.msg9147#new" title="MOVED: I3 flakiness...">MOVED: I3 flakiness...</a>
            #    <br /> on April 11, 2020, 06:24:26 pm
            #  </td>
            my $lastpost = $board->look_down(class => 'lastpost') or die "Cannot get last post info for $board_name.";
            my $last_blob = trim $lastpost->as_text;

            # We assumne the ordering of the hrefs here!
            my $last_post_data = {};
            my $url_count = 0;
            foreach ($lastpost->look_down(_tag => 'a')) {
                my $url = $_->attr('href');
                my $text = $_->as_text;
                if ($url_count == 0) {
                    $last_post_data->{profile_name} = $text;
                    $last_post_data->{profile_url} = $url;
                    $url =~ /profile;u=(\d+)/;
                    $last_post_data->{profile_id} = $1;
                    $profile_data->{$last_post_data->{profile_id}} = get_profile($url);
                } elsif ($url_count == 1) {
                    $last_post_data->{post_title} = $text;
                    $last_post_data->{post_url} = $url;
                    $url =~ /topic=(\d+).msg(\d+)/;
                    ($last_post_data->{post_topic_id}, $last_post_data->{post_message_id}) = ($1, $2);
                }
                $url_count++;
            }
            $last_blob =~ /\s+on\s+(.*)\s+/;
            $last_post_data->{post_date} = $1;

            my $board_data = {
                board_id                => $board_id,
                board_name              => $board_name,
                board_url               => $board_url,
                board_title             => $board_title,
                expected_post_count     => $post_count,
                expected_topic_count    => $topic_count,
                last_post               => $last_post_data,
            };
            $category_data->{$category_id}{$board_id} = $board_data;
        }
    }
    return $category_data;
}

sub get_board_topics {
    my $board_url = shift;
    my $current_page = shift; # pass in 1 to begin

    die "Invalid board URL." if !defined $board_url;
    $board_url =~ s!\.html\?.*$!.html!;
    $board_url =~ s!\.\./smf/!!;

    my $file = "$file_prefix/smf/$board_url";
    warn "File $file not found." if ! -r $file;
    return undef if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $pagelinks = $root->look_down(class => 'pagelinks') or die "No page list found in $file.";

    my $next_page_url = undef;
    my $next_page = undef;

    # If there's no pagelinks section at all, then there must BE only one page.
    if(defined $pagelinks) {
        my @page_links = $pagelinks->look_down(class => 'navPages');

        if((scalar @page_links) < 1) {
            #warn "No navigation links in $board_url";
            #return undef;
        } else {
            foreach my $link (@page_links) {
                my $page_number = $link->as_text;
                next if $page_number <= $current_page;
                $next_page = $page_number;
                $next_page_url = $link->attr('href');
                last;
            }
        }
    }

    # Here, we'd process the topic list on THIS page, and then...
    #
    my $topic_table = $root->look_down(id => 'messageIndex') or die "No topic list found on page $current_page.";
    my $topic_grid = $topic_table->look_down(class => 'table_grid') or die "No topic list found on page $current_page.";
    my $table = $topic_grid->look_down(_tag => 'tbody') or die "No topic list found on page $current_page.";
    my @rows = $table->look_down(_tag => 'tr') or die "No topics found on page $current_page.";
    die "No topic rows found on page $current_page." if (scalar @rows) < 1;
    my @topic_data = ();
    foreach my $row (@rows) {
        my @cols = $row->look_down(_tag => 'td') or die "No topics columns found on page $current_page.";
        die "Expected 5 columns, only got ".(scalar @cols) if (scalar @cols) < 5;

        my $topic_span = $cols[2]->look_down(_tag => 'span') or die "Cannot find message id.";
        my $topic_message_id = $topic_span->attr('id'); # msg_NNN
        $topic_message_id =~ s/msg_//;

        my $topic_started_by_blob = $cols[2]->look_down(_tag => 'p') or die "No topic started by blob.";
        my $topic_started_by_a = $topic_started_by_blob->look_down(_tag => 'a');
        my $topic_started_by_url = "";
        my $topic_started_by_name = "";
        if(defined $topic_started_by_a) {
            $topic_started_by_url = $topic_started_by_a->attr('href');
            $topic_started_by_name = trim $topic_started_by_a->as_text;
        } else {
            # Started by daelaskai
            $topic_started_by_blob->as_text =~ /^Started\s+by\s+(.*)\s*$/;
            $topic_started_by_name = $1;
        }

        my $topic_a = $topic_span->look_down(_tag => 'a') or die "No topic URL.";
        my $topic_title = $topic_a->as_text;
        my $topic_url = $topic_a->attr('href');
        $topic_url =~ /\.html\?topic=(\d+)\.\d+/;
        my $topic_id = $1;

        my $stats_blob = $cols[3]->as_text or warn "Stats not available.";
        $stats_blob =~ /(\d+)\s+Replies\s+(\d+)\s+Views/;
        my ($topic_replies, $topic_views) = ($1,$2);

        my $lastpost = $cols[4];
        my $last_blob = trim $lastpost->as_text;
        my $last_post_data = {};

        my @last_urls = $lastpost->look_down(_tag => 'a'); # should be 2
        $last_post_data->{post_url} = $last_urls[0]->attr('href');
        #index5b35.html?topic=1638.0#msg9143
        $last_post_data->{post_url} =~ /topic=(\d+)\.\d+\#msg(\d+)/;
        ($last_post_data->{post_topic_id}, $last_post_data->{post_message_id}) = ($1,$2);

        if((scalar @last_urls) < 2) {
            # December 16, 2009, 01:49:20 pm by Kalinash
            $last_blob =~ /^(.*?\s+[ap]m)\s+by\s+(.*)\s*$/;
            ($last_post_data->{post_date}, $last_post_data->{profile_name}) = ($1,$2);
            warn "No profile exists for " . $last_post_data->{profile_name};
            $missing_profiles->{$last_post_data->{profile_name}}++;
        } else {
            $last_post_data->{profile_url} = $last_urls[1]->attr('href');
            $last_post_data->{profile_name} = trim $last_urls[1]->as_text;
            $last_blob =~ /^(.*?\s+[ap]m)\s+by/;
            $last_post_data->{post_date} = $1;
            #indexa82d.html?action=profile;u=185
            $last_post_data->{profile_url} =~ /profile;u=(\d+)/;
            $last_post_data->{profile_id} = $1;
            $profile_data->{$last_post_data->{profile_id}} = get_profile($last_post_data->{profile_url});
        }

        push @topic_data, {
            topic_id                => $topic_id,
            topic_message_id        => $topic_message_id,
            topic_title             => $topic_title,
            topic_url               => $topic_url,
            topic_started_by_name   => $topic_started_by_name,
            topic_started_by_url    => $topic_started_by_url,
            topic_replies           => $topic_replies,
            topic_views             => $topic_views,
            last_post               => $last_post_data,
        };
    }

    # At this point, we can either walk down the topic's post chain
    # to collect them all here... or do it later.
    foreach my $topic (@topic_data) {
        my $post_data = get_a_topic($topic->{topic_url}, 1);
        next unless defined $post_data;
        $topic->{posts} = $post_data;
    }

    return {
        current_page            => $current_page,
        current_page_url        => $board_url,
        next_page               => $next_page,
        next_page_url           => $next_page_url,
        topics                  => \@topic_data,
    };
}

sub get_a_topic {
    #index5b35.html?topic=1638.0
    my $topic_url = shift;
    my $current_page = shift; # pass in 1 to begin

    #printf "%s\n", $topic_url;

    die "Invalid topic URL." if !defined $topic_url;
    $topic_url =~ /topic=(\d+)\.\d+$/;
    my $topic_id = $1;
    $topic_url =~ s!\.html\?.*$!.html!;
    $topic_url =~ s!\.\./smf/!!;

    my @posts = ();
    my $next_page_url = undef;
    my $next_page = undef;

    my $file = "$file_prefix/smf/$topic_url";
    die "File $file not found." if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $main_section = $root->look_down(id => 'main_content_section') or die "No main content section in $file.";
    my $pagelinks = $main_section->look_down(class => 'pagelinks floatleft') or die "No pagelinks in $file.";

    # If there's no pagelinks section at all, then there must BE only one page.
    if(defined $pagelinks) {
        my @page_links = $pagelinks->look_down(class => 'navPages');

        foreach my $link (@page_links) {
            my $page_number = $link->as_text;
            next if $page_number <= $current_page;
            $next_page = $page_number;
            $next_page_url = $link->attr('href');
            last;
        }
    }

    my $forumposts = $root->look_down(id => 'forumposts') or die "Can't find any posts in $file.";
    my @post_wrappers = $forumposts->look_down(class => 'post_wrapper');
    return undef unless (scalar @post_wrappers) > 0;
    foreach my $pw (@post_wrappers) {
        my $post = {};

        # poster
        my $poster = $pw->look_down(class => 'poster');
        if(defined $poster) {
            my $profile_a = $poster->look_down(_tag => 'a');
            if(defined $profile_a) {
                $post->{profile_name} = trim $profile_a->as_text;
                my $url = $profile_a->attr('href');
                if(defined $url) {
                    $url =~ /profile;u=(\d+)/;
                    $post->{profile_id} = $1;
                    $profile_data->{$post->{profile_id}} = get_profile($url);
                }
            } else {
                #<div class="poster"><h4> Kalinash </h4><ul class="reset smalltext" id="msg_6848_extra_info"><li class="membergroup">Guest</ul></div>
                my $profile_h4 = $poster->look_down(_tag => 'h4');
                if(defined $profile_h4) {
                    $post->{profile_name} = trim $profile_h4->as_text;
                    warn "No profile exists for " . $post->{profile_name};
                    $missing_profiles->{$post->{profile_name}}++;
                } else {
                    die "No profile a OR h4 found in $file\nwrapper " . $poster->as_HTML;
                }
            }
            my $profile_ul = $poster->look_down(_tag => 'ul');
            if(defined $profile_ul) {
                my $profile_title = $profile_ul->look_down(class => 'title');
                $post->{profile_title} = trim $profile_title->as_text if(defined $profile_title);
                my $profile_group = $profile_ul->look_down(class => 'membergroup');
                $post->{profile_group} = trim $profile_group->as_text if(defined $profile_group);
                my $profile_avatar = $profile_ul->look_down(class => 'avatar');
                if(defined $profile_avatar) {
                    my $profile_avatar_img = $profile_avatar->look_down(_tag => 'img');
                    $post->{avatar_url} = $profile_avatar_img->attr('src') if defined $profile_avatar_img;
                }
                my $profile_postcount = $profile_ul->look_down(class => 'postcount');
                if(defined $profile_postcount) {
                    my $postcount = $profile_postcount->as_text;
                    $postcount =~ /Posts:\s+(\d+)/;
                    $post->{profile_postcount} = $1;
                }
                my $profile_blurb = $profile_ul->look_down(class => 'blurb');
                $post->{profile_blurb} = trim $profile_blurb->as_text if(defined $profile_blurb);
            } else {
                die "No profile ul found in $file\nwrapper " . $poster->as_HTML;
            }
        } else {
            die "No poster found in $file\nwrapper " . $pw->as_HTML;
        }

        # postarea
        my $postarea = $pw->look_down(class => 'postarea');
        my $message_id = undef;
        if(defined $postarea) {
            my $keyinfo = $postarea->look_down(class => 'keyinfo');
            if(defined $keyinfo) {
                my $h5 = $keyinfo->look_down(_tag => 'h5');
                if(defined $h5) {
                    $post->{post_title} = trim $h5->as_text;
                    my $title_a = $h5->look_down(_tag => 'a');
                    if(defined $title_a) {
                        $post->{post_url} = $title_a->attr('href');
                        # index48ec.html?topic=1455.msg7771#msg7771
                        $post->{post_url} =~ /topic=(\d+)\.msg(\d+)\#/;
                        ($post->{topic_id}, $post->{message_id}) = ($1,$2);
                        $message_id = $post->{message_id};
                    }
                }
                my $title_blob = $keyinfo->look_down(class => 'smalltext');
                if(defined $title_blob) {
                    $post->{post_date} = trim $title_blob->as_text;
                    $post->{post_date} =~ /on:\s+(.*[ap]m)/;
                    $post->{post_date} = $1;
                }
            }
        } else {
            die "No post area found in $file\nwrapper " . $pw->as_HTML;
        }

        # post
        my $post_blob = $pw->look_down(class => 'post');
        if(defined $post_blob) {
            my $post_inner = $post_blob->look_down(class => 'inner');
            if(defined $post_inner) {
                if(!defined $message_id) {
                    $message_id = $post_inner->attr('id');
                    $message_id =~ s/^msg_//;
                }
                my $start_tag = $post_inner->starttag('');
                my $end_tag = $post_inner->endtag();
                $post->{post_html} = $post_inner->as_HTML;
                # Hmmmm, do I leave the div tag wrappers, or strip them?
                $post->{post_html} =~ s/^$start_tag//;
                $post->{post_html} =~ s/$end_tag$//;
            } else {
                die "No post inner found in $file\nwrapper " . $post_blob->as_HTML;
            }
        } else {
            die "No post found in $file\nwrapper " . $pw->as_HTML;
        }

        # moderatorbar
        my $moderatorbar = $pw->look_down(class => 'moderatorbar');
        if(defined $moderatorbar and defined $message_id) {
            my $modified_blob = $moderatorbar->look_down(id => "modified_$message_id");
            if(defined $modified_blob) {
                my $modified_date = trim $modified_blob->as_text;
                my $modified_by = undef;
                $modified_date =~ /Edit:\s+(.*[ap]m)\s+by\s+(.*)$/;
                ($modified_date, $modified_by) = ($1, $2);
                $post->{post_modified_date} = $modified_date if defined $modified_date;
                $post->{post_modified_by} = $modified_by if defined $modified_by;
            }
        }

        # done
        #$post->{post_id} = sprintf "%d.%d", $post->{topic_id}, $post->{message_id};
        $post->{post_id} = sprintf "%d", $post->{message_id};
        push @posts, $post->{post_id};
        $posts_by_id->{$post->{post_id}} = $post;
    }

    # At this point, we need to recurse in and do all this again for $next_page_url
    if(defined $next_page and defined $next_page_url) {
        #warn "Found page $next_page! $next_page_url";
        my $results = get_a_topic($next_page_url, $next_page);
        push @posts, @{ $results->{posts} } if defined $results;
    } else {
        #warn "$current_page was the last page.";
        #warn "Next page should have been $next_page." if defined $next_page;
        #warn "Next page should have been $next_page_url." if defined $next_page_url;
    }

    return {
        topic_id    => $topic_id,
        topic_url   => $topic_url,
        post_count  => (scalar @posts),
        posts       => \@posts,
    };
}

sub get_profile {
    my $profile_url = shift;
    die "Invalid profile URL." if !defined $profile_url;
    my $profile = {};

    #indexa82d.html?action=profile;u=185
    $profile->{url} = $profile_url;
    $profile->{url} =~ /profile;u=(\d+)/;
    $profile->{id} = $1;
    $profile->{url} =~ s!\.html\?.*$!.html!;
    $profile->{url} =~ s!\.\./smf/!!;

    my $file = "$file_prefix/smf/" . $profile->{url};
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $profileview = $root->look_down(id => 'profileview') or die "No profileview in $file.";
    my $basicinfo = $profileview->look_down(id => 'basicinfo') or die "No basicinfo in $file.";
    my $username = $basicinfo->look_down(class => 'username') or die "No username in $file.";
    my $user_h4 = $username->look_down(_tag => 'h4') or die "No h4 in $file.";

    $profile->{name} = trim $user_h4->as_text;
    $profile->{name} =~ /^(\S+)\s+(.*)\s*$/;
    ($profile->{name}, $profile->{position}) = ($1,$2);

    my $avatar = $basicinfo->look_down(_tag => 'img') or die "No avatar in $file.";
    $profile->{avatar_url} = $avatar->attr('src');

    my $detailedinfo = $profileview->look_down(id => 'detailedinfo') or die "No detailedinfo in $file.";
    my $content = $detailedinfo->look_down(class => 'content') or die "No content in $file.";
    my $first_dl = $content->look_down(_tag => 'dl') or die "No initial dl in $file.";
    my $second_dl = $content->look_down(class => 'noborder') or die "No second dl in $file.";

    my @first_dt = $first_dl->look_down(_tag => 'dt');
    my @first_dd = $first_dl->look_down(_tag => 'dd');
    for (my $i = 0; $i < (scalar @first_dt); $i++) {
        my $k = trim $first_dt[$i]->as_text;
        my $v = trim $first_dd[$i]->as_text;
        $k =~ s/:\s*$//;
        $k =~ s/\s+/_/g;
        $profile->{lc $k} = $v;
    }

    my @second_dt = $second_dl->look_down(_tag => 'dt');
    my @second_dd = $second_dl->look_down(_tag => 'dd');
    for (my $i = 0; $i < (scalar @second_dt); $i++) {
        my $k = trim $second_dt[$i]->as_text;
        my $v = trim $second_dd[$i]->as_text;
        $k =~ s/:\s*$//;
        $k =~ s/\s+/_/g;
        $profile->{lc $k} = $v;
    }

    my $sig_div = $content->look_down(class => 'signature');
    if(defined $sig_div) {
        my $sig_img = $sig_div->look_down(_tag => 'img');
        if(defined $sig_img) {
            $profile->{signature_url} = $sig_img->attr('src');
        }
    }

    return $profile;
}

my $category_data = get_categories();

# For each board, the url to show topics looks like
# lpmuds.net/smf/indexd0e9.html?board=1.0
# and we can see the 1 is the board ID number
# and the .0 part is the message number to start displaying from...
#
# However, the way this was scraped, the file itself varies...
# lpmuds.net/smf/index318d.html?board=1.20
#
# So, instead, we'll have to harvest the link from the tiny navbar bit.
# There is no "next page" though, so instead we have to know where we are
# via the URL we have, and then divide by 20 and look for the next highest
# number in the Pages: entry
#

# div class="pagelinks floatleft"
#   Pages: [
#       <string>1</strong>
#   ] 
#   a class="navPages" href="url">2</a>
#
# Seems like we either need to search for the newPages links and find one whose
# text is the next number up... if none, then we hit the end.
# The other option is to search the URL part and do (X-1*20).
#

foreach my $category (
    map { $category_data->{$_} } (
        sort { $a <=> $b } keys %$category_data
    )
) {
    foreach my $board (
        map { $category->{$_} } (
            sort { $a <=> $b } grep {!/^category_/} keys %$category
        )
    ) {
        my @topics = ();
        my $url = $board->{board_url};
        my $page = 1;

        while( my $topic_chunk = get_board_topics($url, $page) ) {
            $page = $topic_chunk->{next_page};
            $url = $topic_chunk->{next_page_url};
            push @topics, ( @{ $topic_chunk->{topics} } );
            last if !defined $url;
        }
        #printf "Category %d (%s), Board %d (%s), Topics: %d\n",
        #    $category->{category_id}, $category->{category_name},
        #    $board->{board_id}, $board->{board_name},
        #    (scalar @topics);
        $category_data->{$category->{category_id}}{$board->{board_id}}{topics} = \@topics;
    }
}

my $max_id = 0;
foreach (keys %$profile_data) {
    $max_id = $_ if $_ > $max_id;
}

foreach (keys %$missing_profiles) {
    # Check here to see if a name already exists in the profile data?
    $max_id++;
    $profile_data->{$max_id} = {
        age                 => 'N/A',
        id                  => $max_id,
        name                => $_,
        position            => 'Guest',
        posts               => $missing_profiles->{$_} . " (0.001 per day)",
        date_registered     => "November 30, 2006, 07:18:43 pm",
        last_active         => "November 30, 2006, 07:18:43 pm",
        local_time          => "October 15, 2021, 05:13:18 am",
    };
}

my $json_dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode(
    {
        category_data => $category_data,
        post_data => $posts_by_id,
        profile_data => $profile_data,
    }
);
print "$json_dump\n";

