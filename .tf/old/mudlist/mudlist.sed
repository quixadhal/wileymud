1,/^####/d
/^####/,$d
/^-/d
/^[#-]/!{ N
s/\n/ /g
s///g
s/(.*)//g
s/\[.*\]//g
s/-//g
s/\///g
s/<//g
s/>//g
s/!//g
s/_//g
s/'//g
s/"//g
s/,//g
s/(//g
s/)//g
s/\[//g
s/\]//g
}
