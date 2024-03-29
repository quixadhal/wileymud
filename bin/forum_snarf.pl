#!/usr/bin/perl -w

use strict;
use English qw( −no_match_vars );
use HTML::TreeBuilder;
use JSON qw(encode_json);
use HTML::BBReverse;
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

my $filename_pattern = '/space/stuff/Mirrors/MudMirror-%s/lpmuds.net';
my @data_sets = (qw( 2020-05-11 2021-10-16 ));

my $merged_data = {};
my $profile_data = {};
my $missing_profiles = {};
my $posts_by_id = {};
my $groups = {};

my $output_profiles = {};

sub trim {
    my $str = shift;
    return undef unless defined $str;

    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    return $str;
}

sub process_files {
    my $file_set_number = 0;

    foreach my $file_set (@data_sets) {
        my $filepath = sprintf $filename_pattern, $file_set;
        my $category_data = get_categories($filepath);

        foreach my $category (
            map { $category_data->{$_} } (
                sort { $a <=> $b } keys %$category_data
            )
        ) {
            my $category_id = $category->{category_id};

            foreach my $board (
                map { $category->{$_} } (
                    sort { $a <=> $b } grep {!/^category_/} keys %$category
                )
            ) {
                my $board_id = $board->{board_id};

                my @topics = ();
                my $url = $board->{board_url};
                my $page = 1;

                while( my $topic_chunk = get_board_topics($filepath, $url, $page) ) {
                    $page = $topic_chunk->{next_page};
                    $url = $topic_chunk->{next_page_url};
                    push @topics, ( @{ $topic_chunk->{topics} } );
                    last if !defined $url;
                }
                $category_data->{$category_id}{$board_id}{topics} = \@topics;
            }
        }

        if($file_set_number > 0) {
            # Once we've collected all the data from the new file set
            # we need to merge it into the overall data set.
            foreach my $category (
                map { $category_data->{$_} } (
                    sort { $a <=> $b } keys %$category_data
                )
            ) {
                my $category_id = $category->{category_id};

                foreach my $board (
                    map { $category->{$_} } (
                        sort { $a <=> $b } grep {!/^category_/} keys %$category
                    )
                ) {
                    my $board_id = $board->{board_id};

                    # Make sure the category exists, in case it didn't in the last run.
                    $merged_data->{$category_id} = {}
                        unless exists $merged_data->{$category_id};
                    # Make sure the board exists, in case it didn't in the last run.
                    $merged_data->{$category_id}{$board_id} = {}
                        unless exists $merged_data->{$category_id}{$board_id};

                    # Now, if we want to keep the older data, we skip things that
                    # already exist... if we want to overwrite, we do so.
                    $merged_data->{$category_id}{$board_id}{board_id} = $board_id;

                    $merged_data->{$category_id}{$board_id}{board_name} = $board->{board_name}
                        if !exists $merged_data->{$category_id}{$board_id}{board_name};

                    $merged_data->{$category_id}{$board_id}{board_title} = $board->{board_title}
                        if !exists $merged_data->{$category_id}{$board_id}{board_title};

                    # We want the URL from the most recent data set
                    $merged_data->{$category_id}{$board_id}{board_url} = $board->{board_url};

                    #expected_post_count and expected_topic_count only make sense within
                    #a single data set, so skip both of those.

                    # We want the last_post from the most recent data set
                    $merged_data->{$category_id}{$board_id}{last_post} = $board->{last_post};

                    # At this point, we have to loop through all the topics
                    # and merge posts from the new data set into the old one, if
                    # the topic exists... or just toss the entire topic in if it's new
                    $merged_data->{$category_id}{$board_id}{topics} = []
                        unless exists $merged_data->{$category_id}{$board_id}{topics};

                    # topics are an array, so we may have ordering issues, but we'll do
                    # what we can... missing ones will end up merging in between existing
                    # ones, if they weren't at the bottom of the new set.

                    foreach my $topic (@{ $board->{topics} }) {
                        my $topic_id = $topic->{topic_id};

                        my $found = undef;
                        my $found_index = 0;
                        foreach my $c (keys %$category_data) {
                            foreach my $b (keys %{ $category_data->{$c} }) {
                                foreach my $t (@{ $merged_data->{$c}{$b}{topics} }) {
                                    $found = $t->{posts} if $t->{topic_id} == $topic_id;
                                    last if $found;
                                    $found_index++;
                                }
                                last if $found;
                                $found_index = 0;
                            }
                            last if $found;
                            $found_index = 0;
                        }

                        # OK, if did NOT find the topic, just add it and we're good.
                        # Otherwise, we need to merge fields.
                        if(!$found) {
                            push @{ $merged_data->{$category_id}{$board_id}{topics} }, $topic;
                        } else {
                            # Merge it is!
                            warn "Merging posts for $topic_id";
                            my $merged_topic = {
                                last_post               => $topic->{last_post},
                                posts                   => $found,
                                topic_id                => $topic->{topic_id},
                                topic_message_id        => $topic->{topic_message_id},
                                topic_replies           => $topic->{topic_replies},
                                topic_started_by_name   => $topic->{topic_started_by_name},
                                topic_started_by_url    => $topic->{topic_started_by_url},
                                topic_title             => $topic->{topic_title},
                                topic_url               => $topic->{topic_url},
                                topic_views             => $topic->{topic_views},
                            };

                            $merged_topic->{posts}{topic_url} = $topic->{posts}{topic_url};
                            # Now, we have to add in things from the new posts section.
                            foreach my $p (@{ $topic->{posts}{posts} }) {
                                next if grep {/^$p$/} (@{ $merged_topic->{posts}{posts} });
                                warn "    Merged in post #$p";
                                push @{ $merged_topic->{posts}{posts} }, $p;
                                $merged_topic->{posts}{post_count}++;
                            }

                            warn "Found index == $found_index";
                            $merged_data->{$category_id}{$board_id}{topics}[$found_index] = $merged_topic;
                        }
                    } #topic
                } #board
            } #category
        } #file_set_number > 0

        $file_set_number++;
    }

    # Now, we have ALL the profiles collected, so fill in anyone who's
    # only know by name due to corrupted source data.
    my $max_id = 0;
    foreach (keys %$profile_data) {
        $max_id = $_ if $_ > $max_id;
    }

    foreach my $name (keys %$missing_profiles) {
        # Check here to see if a name already exists in the profile data?
        next if grep {/^$name$/} 
            map { $profile_data->{$_}{name} }
            (keys %$profile_data);
        $max_id++;
        $profile_data->{$max_id} = {
            age                 => 'N/A',
            id                  => $max_id,
            name                => $name,
            position            => 'Guest',
            posts               => $missing_profiles->{$name} . " (0.001 per day)",
            date_registered     => "January 1, 2006, 12:00:00 am",
            last_active         => "January 1, 2006, 12:00:00 am",
            local_time          => "October 15, 2021, 05:13:18 am",
        };
    }

    # At this point, let's gather all the "position" or group data just
    # to have counts in one spot.
    foreach (keys %$profile_data) {
        $groups->{$profile_data->{$_}{position}}++;
    }

    # And finally, JSON encode and output everything
    my $json_dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode({
            merged_data     => $merged_data,
            post_data       => $posts_by_id,
            group_data      => $groups,
            profile_data    => $profile_data,
    });
    print "$json_dump\n";
}

sub get_categories {
    my $file_prefix = shift or die "No file prefix provided.";

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
                    $profile_data->{$last_post_data->{profile_id}} = get_profile($file_prefix, $url);
                } elsif ($url_count == 1) {
                    $last_post_data->{post_title} = $text;
                    $last_post_data->{post_url} = $url;
                    $url =~ /topic=(\d+).msg(\d+)/;
                    ($last_post_data->{post_topic_id}, $last_post_data->{post_message_id}) = ($1, $2);
                }
                $url_count++;
            }
            #$last_blob =~ /\s+on\s+(.*)\s+/;
            #$last_blob =~ /\s+on\s+(.*?\s+\d+,\s+\d+,\s+\d+:\d+:\d+(?:\s+[ap]m)?)\s+/;
            # "post_date" : "line 291: modern ... on May 25, 2020, 03:54:44",
            $last_blob =~ /\s+on\s+((?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec).*)\s+/;
            #$last_post_data->{post_date} = "line 291: $1";
            $last_post_data->{post_date} = $1;
            $last_post_data->{post_date} =~ /(\d+):\d+:\d+$/;
            my $hour = $1;
            $last_post_data->{post_date} .= ($hour < 12) ? " am" : " pm";

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
    my $file_prefix = shift or die "No file prefix provided.";
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
            #$last_blob =~ /^(.*?\s+\d+,\s+\d+,\s+\d+:\d+:\d+(?:\s+[ap]m)?)\s+by\s+(.*)\s*$/;
            #($last_post_data->{post_date}, $last_post_data->{profile_name}) = ("line 397: $1",$2);
            ($last_post_data->{post_date}, $last_post_data->{profile_name}) = ($1,$2);
            warn "$file_prefix - No profile exists for " . $last_post_data->{profile_name} if !exists $missing_profiles->{$last_post_data->{profile_name}};
            $missing_profiles->{$last_post_data->{profile_name}}++;
        } else {
            $last_post_data->{profile_url} = $last_urls[1]->attr('href');
            $last_post_data->{profile_name} = trim $last_urls[1]->as_text;
            $last_blob =~ /^(.*?\s+[ap]m)\s+by/;
            #$last_blob =~ /^(.*?\s+\d+,\s+\d+,\s+\d+:\d+:\d+(?:\s+[ap]m)?)\s+by$/;
            #$last_post_data->{post_date} = "line 405: $1";
            $last_post_data->{post_date} = $1;
            #indexa82d.html?action=profile;u=185
            $last_post_data->{profile_url} =~ /profile;u=(\d+)/;
            $last_post_data->{profile_id} = $1;
            $profile_data->{$last_post_data->{profile_id}} = get_profile($file_prefix, $last_post_data->{profile_url});
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
        my $post_data = get_a_topic($file_prefix, $topic->{topic_url}, 1);
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
    my $file_prefix = shift or die "No file prefix provided.";
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
                    $profile_data->{$post->{profile_id}} = get_profile($file_prefix, $url);
                }
            } else {
                #<div class="poster"><h4> Kalinash </h4><ul class="reset smalltext" id="msg_6848_extra_info"><li class="membergroup">Guest</ul></div>
                my $profile_h4 = $poster->look_down(_tag => 'h4');
                if(defined $profile_h4) {
                    $post->{profile_name} = trim $profile_h4->as_text;
                    warn "$file_prefix - No profile exists for " . $post->{profile_name} if !exists $missing_profiles->{$post->{profile_name}};
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
                    #$post->{post_date} =~ /on:\s+(.*?\s+\d+,\s+\d+,\s+\d+:\d+:\d+(?:\s+[ap]m)?)/;
                    #$post->{post_date} = "line 557: $1";
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
                my $bbr = HTML::BBReverse->new(reverse_for_edit => 0);
                $post->{post_bbcode} = $bbr->reverse($post->{post_html});
                $post->{post_bbcode} =~ s/<\s*?br\s*?\/?>/[br]/gi;
                $post->{post_bbcode} =~ s/<\s*?\/br\s*?>/[\/br]/gi;
                $post->{post_bbcode} =~ s/<\s*?hr\s*?\/?>/[hr]/gi;
                $post->{post_bbcode} =~ s/<\s*?\/hr\s*?>/[\/hr]/gi;
                $post->{post_bbcode} =~ s/<\s*?p\s*?\/?>/[p]/gi;
                $post->{post_bbcode} =~ s/<\s*?\/p\s*?>/[\/p]/gi;
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
                #$modified_date =~ /Edit:\s+(.*?\s+\d+,\s+\d+,\s+\d+:\d+:\d+(?:\s+[ap]m)?)\s+by\s+(.*)$/;
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
        my $results = get_a_topic($file_prefix, $next_page_url, $next_page);
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
    my $file_prefix = shift or die "No file prefix provided.";
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

process_files();

# To prepare for data ouput, we need to group things a bit.
#
# We first need to map SMF user groups to the target user groups.
#
# Then we need to map user profiles.
#
# Then we figure out the category/board/topic/post hierarchy and adjust that.


# So, now that we have phpBB running, there's a user_add() funciton in it.
# Looks lke it requires: username, group_id, user_email, and user_type.
#       forum/includes/functions_user.php
#
# Required values
#
#       'username'                  => $user_row['username'],
#       'username_clean'            => $username_clean,
#       'user_password'             => (isset($user_row['user_password'])) ? $user_row['user_password'] : '',
#       'user_email'                => strtolower($user_row['user_email']),
#       'group_id'                  => $user_row['group_id'],
#       'user_type'                 => $user_row['user_type'],
#
# Then there are lots of optional values
#
#       'user_permissions'          => '',
#       'user_timezone'             => $config['board_timezone'],
#       'user_dateformat'           => $config['default_dateformat'],
#       'user_lang'                 => $config['default_lang'],
#       'user_style'                => (int) $config['default_style'],
#       'user_actkey'               => '',
#       'user_ip'                   => '',
#       'user_regdate'              => time(),
#       'user_passchg'              => time(),
#       'user_options'              => 230271,
#       // We do not set the new flag here - registration scripts need to specify it
#       'user_new'                  => 0,
#
#       'user_inactive_reason'      => 0,
#       'user_inactive_time'        => 0,
#       'user_lastmark'             => time(),
#       'user_lastvisit'            => 0,
#       'user_lastpost_time'        => 0,
#       'user_lastpage'             => '',
#       'user_posts'                => 0,
#       'user_colour'               => '',
#       'user_avatar'               => '',
#       'user_avatar_type'          => '',
#       'user_avatar_width'         => 0,
#       'user_avatar_height'        => 0,
#       'user_new_privmsg'          => 0,
#       'user_unread_privmsg'       => 0,
#       'user_last_privmsg'         => 0,
#       'user_message_rules'        => 0,
#       'user_full_folder'          => PRIVMSGS_NO_BOX,
#       'user_emailtime'            => 0,
#
#       'user_notify'               => 0,
#       'user_notify_pm'            => 1,
#       'user_notify_type'          => NOTIFY_EMAIL,
#       'user_allow_pm'             => 1,
#       'user_allow_viewonline'     => 1,
#       'user_allow_viewemail'      => 1,
#       'user_allow_massemail'      => 1,
#
#       'user_sig'                  => '',
#       'user_sig_bbcode_uid'       => '',
#       'user_sig_bbcode_bitfield'  => '',
#
#       'user_form_salt'            => unique_id(),
#

# What we've harvested from the dump...
#
#       "age" : "49",
#       "avatar_url" : "http://ebspso.dnsalias.org/lpmuds/images/avatar_80y.png",
#    or "avatar_url" : "../forum/Themes/default/images/useroff.gif",
#       "date_registered" : "N/A",
#    or "date_registered" : "January 01, 2007, 11:29:30 am",
#       "gender" : "Male",
#       "id" : "10",
#       "last_active" : "April 09, 2014, 03:37:55 pm",
#       "local_time" : "October 15, 2021, 08:01:09 am",
#       "location" : "UK",
#       "name" : "Tricky",
#       "personal_text" : "I like what I code and I code what I like!",
#       "position" : "BFF",
#       "posts" : "189 (N/A per day)",
#       "url" : "index7e35.html"

# So, remap....
#       id          -> user_id (but this is returned by user_add)
#       name        -> username
#       position    -> group_id ?
#       avatar_url  -> user_avatar + user_avatar_type + 
#                      user_avatar_width + user_avatar_height
#       last_active -> user_lastpost_time (epoch)
#
# And we don't appar to care about gender, birthday, personal text
# We'll have to fudge an email address and figue otu what user_type is.
# I also think we'll probably have to use last_active for the user_regdate.

# The forum software uses a utf8_clean_string() wrapper for usernames, and it
# keeps the text entererd for display purposes.  I suspect we have the display
# text, since we scraped the data from the HTML side.  Thus, we may need to
# replicate the forum's php function here...
#
# includes/utf/utf_tools.php
#




