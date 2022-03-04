#!/usr/bin/perl -w

use strict;
use English qw( −no_match_vars );
use HTML::TreeBuilder;
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

#my $file_prefix = '/space/stuff/Mirrors/MudMirror-2021-10-16/lpmuds.net';
my $file_prefix = '/space/stuff/Mirrors/MudMirror-2020-05-11/lpmuds.net';

sub trim {
    my $str = shift;
    return undef unless defined $str;

    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    return $str;
}

sub get_boards {
    my $file = "$file_prefix/forum/index.html";
    die "File $file not found." if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $table = $root->look_down(class => 'table_list') or die "No table_list found in $file.";

    my @categories = $table->look_down(id => qr/category_\d+_boards/);
    die "No categories found in $file." if (scalar @categories) < 1;
    my $board_results = {};
    foreach my $cat (@categories) {
        my $cat_id = $cat->attr('id');
        $cat_id =~ /category_(\d+)_boards/;
        my $cat_number = $1;
        my $parent = $cat->look_up(class => 'table_list') or die "Cannot find table list for category $cat_id.";
        my $cat_head = $parent->look_down(id => "category_$cat_number") or die "Cannot find category_$cat_number.";
        my $cat_info = $cat_head->look_down(class => 'catbg') or die "Cannot get category name for $cat_id.";
        my $cat_name = trim $cat_info->as_text;

        $board_results->{$cat_number} = {
            category_id     => $cat_number,
            category_name   => $cat_name,
        };

        my @boards = $cat->look_down(id => qr/board_\d+/);
        die "No boards found in category $cat_name." if (scalar @boards) < 1;
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

            my $info = $board->look_down(class => 'info') or die "Cannot get info for board $board_id.";
            #<td class="info">
            #  <a class="subject" 
            #  href="../smf/indexa950.html?PHPSESSID=atphbp79901okvnth7h9oh9jq8&amp;board=1.0" name="b1">
            #  General</a>
            #  <p>A forum for general LPC, LPmud, mostly-on-topic stuff.
            #</td>

            my $anchor = $info->look_down(class => 'subject') or die "Cannot get url for board $board_id.";
            my $board_url = $anchor->attr('href');
            my $board_name = trim $anchor->as_text;

            my $p = $info->look_down(_tag => 'p') or die "Cannot get board description for $board_name.";
            my $board_desc = trim $p->as_text;

            #  <td class="stats windowbg">
            #    <p>1621 Posts <br /> 271 Topics
            #  </td>
            my $stats = $board->look_down(class => 'stats windowbg') or die "Cannot get stats for $board_name.";
            my $p2 = $stats->look_down(_tag => 'p') or die "Cannot get stats for $board_name.";
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
            my $lastpost = $board->look_down(class => 'lastpost') or die "Cannot get last post info for $board_name.";
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

sub get_board_topics {
    my $board_url = shift;
    my $current_page = shift; # pass in 1 to begin
    $board_url =~ s!\.html\?.*$!.html!;
    $board_url =~ s!\.\./smf/!!;

    my $file = "$file_prefix/smf/$board_url";
    die "File $file not found." if ! -r $file;
    my $root = HTML::TreeBuilder->new_from_file($file) or die "Can't parse $file.";
    my $pagelinks = $root->look_down(class => 'pagelinks') or die "No page list found in $file.";
    my @page_links = $pagelinks->look_down(class => 'navPages');
    my $next_page_url = undef;
    my $next_page = undef;

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
            #printf "TOPIC in $file STARTED BY BLOB: %s\n", $topic_started_by_blob->as_text;
            # Started by daelaskai
            $topic_started_by_blob->as_text =~ /^Started\s+by\s+(.*)\s*$/;
            $topic_started_by_name = $1;
        }

        my $topic_a = $topic_span->look_down(_tag => 'a') or die "No topic URL.";
        my $topic_desc = $topic_a->as_text;
        my $topic_url = $topic_a->attr('href');
        $topic_url =~ /\.html\?topic=(\d+)\.\d+/;
        my $topic_id = $1;

        my $stats_blob = $cols[3]->as_text or warn "Stats not available.";
        $stats_blob =~ /(\d+)\s+Replies\s+(\d+)\s+Views/;
        my ($topic_replies, $topic_views) = ($1,$2);

        my $lastpost = $cols[4];
        my $last_blob = trim $lastpost->as_text;
        my @last_urls = $lastpost->look_down(_tag => 'a'); # should be 2
        my $last_post_url = $last_urls[0]->attr('href');
        my $last_post_profile_url = "";
        my $last_post_profile_name = "";
        my $last_post_date = "";
        if((scalar @last_urls) < 2) {
            #printf "LAST BLOB in $file: %s\n", $lastpost->as_text;
            # December 16, 2009, 01:49:20 pm by Kalinash
            $last_blob =~ /^(.*?\s+[ap]m)\s+by\s+(.*)\s*$/;
            ($last_post_profile_name, $last_post_date) = ($1,$2);
        } else {
            $last_post_profile_url = $last_urls[1]->attr('href');
            $last_post_profile_name = trim $last_urls[1]->as_text;
            $last_blob =~ /^(.*?\s+[ap]m)\s+by/;
            $last_post_date = $1;
        }

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

    return {
        current_page            => $current_page,
        current_page_url        => $board_url,
        next_page               => $next_page,
        next_page_url           => $next_page_url,
        topics                  => \@topic_data,
    };
}

sub show_board_data {
    my $board_results = shift;

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
}

my $board_results = get_boards();

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

my $topic_results = {};

foreach my $category (
    map { $board_results->{$_} } (
        sort { $a <=> $b } keys %$board_results
    )
) {
    $topic_results->{$category->{category_id}} = {};

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
        printf "Category %d (%s), Board %d (%s), Topics: %d\n",
            $category->{category_id}, $category->{category_name},
            $board->{board_id}, $board->{board_name},
            (scalar @topics);
        $topic_results->{$category->{category_id}}{$board->{board_id}} = \@topics;
    }
}

print Dumper($topic_results);

# Interesting.. /space/stuff/Mirrors/MudMirror-2020-05-11/lpmuds.net/smf/index0e50.html
# Looks like the format changed?  We may have to adapt, as the forum might have started
# using a newer format for the last entries...
#
