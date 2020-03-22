# this can log into facebook and can verify that it worked. Need to implement these two cURL requests.

EMAIL='email@verizon.net' # edit this
PASS='password' # edit this

COOKIES='cookies.txt'
USER_AGENT='python-requests/2.18.4'

curl -X POST 'https://m.facebook.com/login.php' --verbose --user-agent $USER_AGENT --data-urlencode "email=${EMAIL}" --data-urlencode "pass=${PASS}" --cookie $COOKIES --cookie-jar $COOKIES

curl -X GET 'https://m.facebook.com/home.php' --verbose --user-agent $USER_AGENT --cookie $COOKIES --cookie-jar $COOKIES