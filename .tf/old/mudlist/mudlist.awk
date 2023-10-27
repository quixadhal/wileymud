{
  if(NF > 4) {
    valid["tiny"] = "tiny";
    valid["muck"] = "tiny";
    valid["mush"] = "tiny";
    valid["muse"] = "tiny";
    valid["teeny"] = "tiny";
    valid["lp"] = "lp";
    valid["dgd"] = "lp";
    valid["lp_german"] = "lp";
    valid["diku"] = "diku";
    valid["rom"] = "diku";
    valid["circle"] = "diku";
    valid["silly"] = "diku";
    valid["lpp"] = "lpp";

    if(valid[tolower($NF)] == "") {
      printf("/addtelnet ");
    } else {
      printf("/add%s ", tolower(valid[tolower($NF)]));
    }
    for(i= 1; i< (NF-5); i++) {
      printf("%s", tolower($i));
    }
    printf("%s %s %s\n", tolower($(NF-5)), tolower($(NF-3)), $(NF-2));
  }
}
