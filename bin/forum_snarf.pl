#!/usr/bin/perl -w

use strict;
use HTML::TreeBuilder;
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

sub trim {
    my $str = shift;
    return undef unless defined $str;

    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    return $str;
}

sub get_boards {
    my $file = '/space/stuff/Mirrors/MudMirror-2021-10-16/lpmuds.net/forum/index.html';
    die "File not found." if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse file.";
    my $table = $root->look_down(class => 'table_list') or die "No table_list found.";


    my @categories = $table->look_down(id => qr/category_\d+_boards/);
    my $board_results = {};
    foreach my $cat (@categories) {
        my $cat_id = $cat->attr('id');
        $cat_id =~ /category_(\d+)_boards/;
        my $cat_number = $1;
        my $parent = $cat->look_up(class => 'table_list');
        my $cat_head = $parent->look_down(id => "category_$cat_number");
        my $cat_info = $cat_head->look_down(class => 'catbg');
        my $cat_name = trim $cat_info->as_text;

        $board_results->{$cat_number} = {
            category_id     => $cat_number,
            category_name   => $cat_name,
        };

        my @boards = $cat->look_down(id => qr/board_\d+/);
        foreach my $board (@boards) {
            my $board_id = $board->attr('id');
            $board_id =~ /board_(\d+)/;
            my $board_number = $1;

            #<tr class="windowbg2" id="board_1">
            #  <td class="icon windowbg">
            #    <a href="../smf/indexa950.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;board=1.0">
            #    <img alt="No New Posts" src="Themes/default/images/off.png" title="No New Posts" />
            #    </a>
            #  </td>
            #  <td class="info">
            #    <a class="subject" href="../smf/indexa950.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;board=1.0" name="b1">General
            #    </a>
            #    <p>A forum for general LPC, LPmud, mostly-on-topic stuff.
            #  </td>
            #  <td class="stats windowbg">
            #    <p>1621 Posts <br /> 271 Topics
            #  </td>
            #  <td class="lastpost">
            #    <p><strong>Last post</strong> by
            #    <a href="../smf/index8615.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;action=profile;u=312">Adam</a>
            #    <br /> in 
            #    <a href="../smf/index7910.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;topic=1641.msg9147#new" title="MOVED: I3 flakiness...">MOVED: I3 flakiness...</a>
            #    <br /> on April 11, 2020, 06:24:26 pm
            #  </td>
            #</tr>

            my $info = $board->look_down(class => 'info');
            #<td class="info">
            #  <a class="subject" 
            #  href="../smf/indexa950.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;board=1.0" name="b1">
            #  General</a>
            #  <p>A forum for general LPC, LPmud, mostly-on-topic stuff.
            #</td>

            my $anchor = $info->look_down(class => 'subject');
            my $board_url = $anchor->attr('href');
            my $board_name = trim $anchor->as_text;

            my $p = $info->look_down(_tag => 'p');
            my $board_desc = trim $p->as_text;

            #  <td class="stats windowbg">
            #    <p>1621 Posts <br /> 271 Topics
            #  </td>
            my $stats = $board->look_down(class => 'stats windowbg');
            my $p2 = $stats->look_down(_tag => 'p');
            my $counts = $p2->as_text;
            $counts =~ /(\d+)\s+Posts\s+(\d+)\s+Topics/;
            my ($posts, $topics) = ($1, $2);

            #  <td class="lastpost">
            #    <p><strong>Last post</strong> by
            #    <a href="../smf/index8615.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;action=profile;u=312">Adam</a>
            #    <br /> in 
            #    <a href="../smf/index7910.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;topic=1641.msg9147#new" title="MOVED: I3 flakiness...">MOVED: I3 flakiness...</a>
            #    <br /> on April 11, 2020, 06:24:26 pm
            #  </td>
            my $lastpost = $board->look_down(class => 'lastpost');
            my $blah = $lastpost->as_text;

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
                } elsif ($url_count == 1) {
                    $last_post_data->{post_title} = $text;
                    $last_post_data->{post_url} = $url;
                    $url =~ /topic=(\d+).msg(\d+)/;
                    ($last_post_data->{post_topic_id}, $last_post_data->{post_message_id}) = ($1, $2);
                }
                $url_count++;
            }
            $blah =~ /\s+on\s+(.*)\s+/;
            $last_post_data->{post_date} = $1;

            my $data = {
                board_id    => $board_number,
                board_name  => $board_name,
                board_url   => $board_url,
                board_desc  => $board_desc,
                posts       => $posts,
                topics      => $topics,
                last_post   => $last_post_data,
            };
            $board_results->{$cat_number}{$board_number} = $data;
        }
    }
    return $board_results;
}

my $board_results = get_boards();
#print Dumper($board_results);

foreach my $category (
    map { $board_results->{$_} } (
        sort { $a <=> $b } keys %$board_results
    )
) {
    # insert into categories (id, name) values (?,?)
    printf "Category            %02d\n", $category->{category_id};
    printf "    NAME            %s\n", $category->{category_name};
    foreach my $board (
        map { $category->{$_} } (
            sort { $a <=> $b } grep {!/^category_/} keys %$category
        )
    ) {
        # insert into categories_boards (category_id, board_id) values (?,?)
        # insert into boards (id, name, url, desc) values (?,?,?,?)
        printf "    Board           %02d\n", $board->{board_id};
        printf "        NAME        %s\n", $board->{board_name};
        printf "        DESC        %s\n", $board->{board_desc};
        printf "        TOPICS      %d\n", $board->{topics};
        printf "        POSTS       %d\n", $board->{posts};
        printf "        LAST POST   %s\n", $board->{last_post}{post_date};
        printf "            Author  %s (user %d)\n",
            $board->{last_post}{profile_name},
            $board->{last_post}{profile_id};
        printf "            Post    %s (topic %d, message %d)\n",
            $board->{last_post}{post_title},
            $board->{last_post}{post_topic_id},
            $board->{last_post}{post_message_id};
    }
}


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
    map { $board_results->{$_} } (
        sort { $a <=> $b } keys %$board_results
    )
) {
    foreach my $board (
        map { $category->{$_} } (
            sort { $a <=> $b } grep {!/^category_/} keys %$category
        )
    ) {
        my $board_id = $board->{board_id};
        my $board_url = $board->{board_url};
        $board_url =~ s!\.html\?.*$!.html!;

        my $current_page = 1;
        my $file = "/space/stuff/Mirrors/MudMirror-2021-10-16/lpmuds.net/forum/$board_url";
        print "FILE: $file\n";
        die "File not found." if ! -r $file;
        my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse file.";
        my $pagelinks = $root->look_down(class => 'pagelinks') or die "No page list found.";
        my @page_links = $pagelinks->look_down(class => 'navPages');
        my $next_page_url = undef;
        my $next_page = undef;

        foreach my $link (@page_links) {
            my $page_number = $link->as_text;
            next if $page_number <= $current_page;
            $next_page = $page_number;
            $next_page_url = $link->attr('href');
            last;
        }

        print "Current Page:    $current_page\n";
        print "Next Page:       $next_page\n"       if defined $next_page;
        print "URL:             $next_page_url\n"   if defined $next_page_url;

        # Here, we'd process the topic list on THIS page, and then...
        #
        my $topic_table = $root->look_down(id => 'messageIndex') or die "No topic list found.";
        my $topic_grid = $topic_table->look_down(class => 'table_grid') or die "No topic list found.";
        my $table = $topic_grid->look_down(_tag => 'tbody') or die "No topic list found.";
        my @rows = $table->look_down(_tag => 'tr') or die "No topics found.";
        my @topic_data = ();
        foreach my $row (@rows) {
            my @cols = $row->look_down(_tag => 'td') or die "No topics columns found.";

            my $topic_span = $cols[2]->look_down(_tag => 'span');
            my $topic_message_id = $topic_span->attr('id'); # msg_NNN
            $topic_message_id =~ s/msg_//;

            my $topic_started_blob = $cols[2]->look_down(_tag => 'p');
            my $topic_started_a = $topic_started_blob->look_down(_tag => 'a');
            my $topic_started_by_url = $topic_started_a->attr('href');
            my $topic_started_by_name = $topic_started_a->as_text;

            my $topic_a = $topic_span->look_down(_tag => 'a');
            my $topic_desc = $topic_a->as_text;
            my $topic_url = $topic_a->attr('href');
            $topic_url =~ /\.html\?topic=(\d+)\.\d+/;
            my $topic_id = $1;

            my $stats_blob = $cols[3]->as_text;
            $stats_blob =~ /(\d+)\s+Replies\s+(\d+)\s+Views/;
            my ($topic_replies, $topic_views) = ($1,$2);

            my $lastpost = $cols[4];
            my $last_blob = trim $lastpost->as_text;
            my @last_urls = $lastpost->look_down(_tag => 'a'); # should be 2
            my $last_post_url = $last_urls[0]->attr('href');
            my $last_post_profile_url = $last_urls[1]->attr('href');
            my $last_post_profile_name = $last_urls[1]->as_text;

            $last_blob =~ /^(.*?\s+[ap]m)\s+by/;
            my $last_post_date  = $1;

            push @topic_data, {
                topic_id                => $topic_id,
                topic_message_id        => $topic_message_id,
                topic_desc              => $topic_desc,
                topic_url               => $topic_url,
                topic_started_by_name   => $topic_started_by_name,
                topic_started_by_url    => $topic_started_by_url,
                topic_replies           => $topic_replies,
                topic_views             => $topic_views,
                last_post_url           => $last_post_url,
                last_post_date          => $last_post_date,
                last_post_profile_name  => $last_post_profile_name,
                last_post_profile_url   => $last_post_profile_url,
            };
        }

        print Dumper(\@topic_data);

        # At this point, we'd walk down that page to get the next page, etc, etc.
        exit 1;

    }
}

