{
  if(world[$2] == "") {
    world[$2] = 1;
    print $0
  } else {
    printf("%s %s2 %s %s\n", $1, $2, $3, $4);
  }
}
