ln -sf /usr/share/dict/words
sort -R < words | head -5 | sort -d > randsort5words.txt
