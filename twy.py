from twython import Twython, TwythonError
import warnings
warnings.filterwarnings('ignore')
APP_KEY = "mDwWhht12raixNyj0d42693or"
APP_SECRET = "mfMM5k6rA6ASUEx1ZakhiOJndaqdWdDkdTYxkYW7pyPe7ohISz"

OAUTH_TOKEN = '3685938257-NONwtBSYo6OBFL0Q2tI56fOr1ypMpreks1xMDq2'
OAUTH_TOKEN_SECRET ='6tUM29hTOLJJiAc5u3Xp485alhMq4AxsM7grO8nrChTVG'

twitter = Twython(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
trends = twitter.get_place_trends(id=721943) #trends for rome
#print trends[0]['trends'][0]['name']
with open("trend.txt","w") as f:
	f.write(trends[0]['trends'][0]['name'])
