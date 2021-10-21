ls /home
echo "238P is cool" > x.txt
cat < x.txt
rm redir.out
echo "hello from test.sh" | wc -c > redir.out
ls | sort | uniq | wc -c
ls /home | sort | uniq | wc -l
rm usernames.txt
ls /home > usernames.txt
rm users_with_v.txt
cat < usernames.txt | sort | uniq | grep v > users_with_v.txt
echo usernames with v:
cat users_with_v.txt