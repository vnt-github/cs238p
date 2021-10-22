ls /home
rm -rf x.txt
echo "238P is cool" > x.txt
cat < x.txt
rm -rf redir.out
echo "hello from test.sh" | wc -c > redir.out
cat redir.out
ls | sort | uniq | wc -c
ls /home | sort | uniq | wc -l
rm -rf usernames.txt
ls /home > usernames.txt
rm -rf users_with_v.txt
cat < usernames.txt | sort | uniq | grep v > users_with_v.txt
echo usernames with v:
cat users_with_v.txt
cd /home/vbharot/cs238p/discussions/3/new_folder
echo "current location"
pwd
cd ..
echo "current location"
pwd
history | grep txt
history > his.txt
cat < his.txt
echo 1 ; echo 2 ; echo 3 ; echo all done
ls > lis.txt ; history > his.txt ; wc < lis.txt ; wc < his.txt ; history | grep echo ; echo done
