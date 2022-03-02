#!/usr/bin/perl -w

use strict;
use HTML::TreeBuilder;
use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

sub get_boards {
    my $file = '/space/stuff/Mirrors/MudMirror-2020-05-11/lpmuds.net/forum/index.html';
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
        my $cat_name = $cat_info->as_text;
        $cat_name =~ s/^\s+//;
        $cat_name =~ s/\s+$//;

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
            my $board_name = $anchor->as_text;
            $board_name =~ s/^\s+//;
            $board_name =~ s/\s+$//;

            my $p = $info->look_down(_tag => 'p');
            my $board_desc = $p->as_text;
            $board_desc =~ s/^\s+//;
            $board_desc =~ s/\s+$//;

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

