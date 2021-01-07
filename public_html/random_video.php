<?php
    $playlist_list = array(
        'https://www.youtube.com/watch?v=XGGYXoGrLaM&list=RDGMEMXdNDEg4wQ96My0DhjI-cIgVMXGGYXoGrLaM&start_radio=1', // Buono
        'https://www.youtube.com/watch?v=pFrKZxStL_U&list=RDCMUCiS882YPwZt1NfaM0gR0D9Q&start_radio=1',              // Genshin Impact
        'https://www.youtube.com/watch?v=BBzOvDz-HWU&list=RDBBzOvDz-HWU&start_radio=1',                             // Sleep Wonder
        'https://www.youtube.com/watch?v=0rg4VqKDgSA&list=RD0rg4VqKDgSA&start_radio=1',                             // Atelier Ryza
        'https://www.youtube.com/watch?v=ap5p81yATDo&list=RDap5p81yATDo&start_radio=1',                             // ClariS
        'https://www.youtube.com/watch?v=58T3l6NFUD8&list=RDMM58T3l6NFUD8&start_radio=1',                           // Walkure
        'https://www.youtube.com/watch?v=GiKcLCUy1Yc&list=RDGiKcLCUy1Yc&start_radio=1',                             // Buono
        'https://www.youtube.com/watch?v=_abj2xbf2mA&list=RD_abj2xbf2mA&start_radio=1',                             // I J I
        'https://www.youtube.com/watch?v=K1PCl5D-IpU&list=RDK1PCl5D-IpU&start_radio=1',                             // Bunny Girl Sepnai
        'https://www.youtube.com/watch?v=yPWCFWKAgRA&list=RDyPWCFWKAgRA&start_radio=1',                             // Trysail
        'https://www.youtube.com/watch?v=xJwP2goQDJM&list=RDxJwP2goQDJM&start_radio=1',                             // Pastel Wind
        'https://www.youtube.com/watch?v=HkiJwtU8zMQ&list=RDHkiJwtU8zMQ&start_radio=1',                             // otomegokoro
        'https://www.youtube.com/watch?v=uZ11JUZM8V0&list=RDuZ11JUZM8V0&start_radio=1',                             // syncopation
        'https://www.youtube.com/watch?v=Ro-_cbfdrYE&list=RDRo-_cbfdrYE&start_radio=1',                             // Ijime, dame, zettai
        'https://www.youtube.com/watch?v=Tz-63JEkVeE&list=RDTz-63JEkVeE&start_radio=1',                             // Girls und Panzer wind
        'https://www.youtube.com/watch?v=iI-BtRbhulE&list=RDiI-BtRbhulE&start_radio=1',                             // Tokkou Dance
        'https://www.youtube.com/watch?v=GoqcIQ37amo&list=RDGoqcIQ37amo&start_radio=1',                             // Brown Sabbath
        'https://www.youtube.com/watch?v=2CZ_b6NbFMc&list=RD2CZ_b6NbFMc&start_radio=1',                             // Brown Sabbath
        'https://www.youtube.com/watch?v=sdPwvBJiQJY&list=RDsdPwvBJiQJY&start_radio=1',                             // Girls Dead Monster
        'https://www.youtube.com/watch?v=L_iOcKZTJzw&list=RDL_iOcKZTJzw&start_radio=1',                             // Girls und Panzer
        'https://www.youtube.com/watch?v=8F7VGrix2L0&list=RD8F7VGrix2L0&start_radio=1',                             // Atelier Ryza 2
        'https://www.youtube.com/watch?v=HQbMEgfo5To&list=RDHQbMEgfo5To&start_radio=1',                             // Space Battleship Yamato
        'https://www.youtube.com/watch?v=ZVJ3Ho83Ksg&list=RDZVJ3Ho83Ksg&start_radio=1',                             // Kyoto Tachibana Band
        'https://www.youtube.com/watch?v=QZShA_a-5r8&list=RDQZShA_a-5r8&start_radio=1',                             // Nyango Star
        'https://www.youtube.com/watch?v=oA6UYTbOEMQ&list=RDoA6UYTbOEMQ&start_radio=1',                             // Ladybaby
        'https://www.youtube.com/watch?v=DgTEKoYhAXg&list=RDDgTEKoYhAXg&start_radio=1',                             // Ningen Isu
        'https://www.youtube.com/watch?v=Jfk4WbW5lp0&list=RDJfk4WbW5lp0&start_radio=1',                             // Vitamin Love
        'https://www.youtube.com/watch?v=qqKG7I6oBnc&list=RDqqKG7I6oBnc&start_radio=1',                             // Suzuki Airi
        'https://www.youtube.com/watch?v=I3Yy6c0Tlvk&list=RDI3Yy6c0Tlvk&start_radio=1',                             // Arcadia
        'https://www.youtube.com/watch?v=OPCiC1Nku18&list=RDOPCiC1Nku18&start_radio=1',                             // Jefferson Starship
        'https://www.youtube.com/watch?v=rrWXFxLIV3w&list=RDrrWXFxLIV3w&start_radio=1',                             // Hall and Oates
        'https://www.youtube.com/watch?v=O9VOud8y3tg',                                                              // Zombie Land Saga opening
        'https://www.youtube.com/watch?v=Turf7WDB3iY',                                                              // Angel Beats opening
        'https://www.youtube.com/watch?v=TfXSBBIzRyg'                                                               // Girls und Panzer opening
    );
    $url = sprintf("Location: %s", $playlist_list[array_rand($playlist_list)]);
    header($url);
    die("You are no fun.");
?>
