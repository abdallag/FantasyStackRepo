import requests
import os
import html
import re
import json
import pandas as pd

import save

cur_start = 20

def download():
    print ("Downloading...")
    params = {
        "orderBy" : "points",
        "focus" : "range",
        "positions" : "1,2,3,4",
        "min_cost" : "35",
        "max_cost" : "130",
        "search_term" : "",
        "gw_start" : "1",
        "gw_end" : "4",
        "first" : "0",
        "last" : "700",
        "season" : "20" + str(cur_start)
        }
    url = "https://data.fantasyfootballhub.co.uk/api/player-predictions/"

    headers = {
       "Connection" : "keep-alive",
      "Cache-Control" : "max-age=0",
      "Upgrade-Insecure-Requests" : "1",
      "User-Agent" : "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.102 Safari/537.36",
      "Accept" : "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9",
      "Sec-Fetch-Site" : "none",
      "Sec-Fetch-Mode" : "navigate",
      "Sec-Fetch-User" : "?1",
      "Sec-Fetch-Dest" : "document",
      "Accept-Language" : "en-US,en;q=0.9,ar;q=0.8",
    }

    cookiefile = open(os.environ['USERPROFILE'] + "\\ffhcookie" )
    cookie = cookiefile.readline()

    resp = requests.get(url=url, params=params, headers = headers, cookies = {"_rdt_uuid": cookie})

    if resp.status_code != 200:
        print( "Error downloading data!")
        print( "Error code = " + str(resp.status_code))
        print( "Error message = ")
        print( resp.content)
        return

    print ("Downloaded succesfully!")

    return html.unescape(resp.content.decode('unicode_escape'))


def get_json(htmlfile):
    print("Extracting JSON...")
    out = ["["]
    found = False

    for l in htmlfile.splitlines():
      if re.match("^[\s]+{", l) != None:
        found = True
 
      if found:
        if re.match("^]",l) != None:
          out.append("]")
          break
        out.append(l)
    return ''.join(out)


def get_season_df(jdata):
    print ("Processing season DataFrame...")
    newdf = pd.DataFrame()
    
    for i in range(len(jdata)):
      p = jdata[i]
      row = pd.DataFrame([[ p['search_term'], p["player"]["position_id"], int(float(p["season_prediction"])), p["player"]["team"], int(float(p["season_prediction"])), p["player"]["now_cost"]]],
                         columns=[ "player", "element_type", "total_points", "team_code", "prev_points", "value"],
                         index = [i])
      newdf = pd.concat([newdf, row])

    newdf['player'] = newdf.player.str.replace(' ', '-')

    return newdf

def get_gw_df(jdata):
    print ("Processing week DataFrame...")
    dfarray = [pd.DataFrame()]
    for i in range(1,39):
        dfarray.append(pd.DataFrame())
  
    k = 0
    for i in range(len(jdata)):
        p = jdata[i]
        for predict in p['predictions']:
            if predict['predicted_pts'] != None:
                row = pd.DataFrame([[ predict['gw'], p['player']['first_name'], p['player']['second_name'] , 0 , 0, int(100 * float(predict['predicted_pts']))]],
                                columns=[ "GW", "first_name", "second_name", "total_points", "value", "xpts"],
                                index = [k])
                k = k + 1
                gw = int(predict['gw'])
                dfarray[gw] = pd.concat([dfarray[gw], row])
    for i in range(1,39):
        if not dfarray[i].empty:
            dfarray[i]['player'] = dfarray[i].first_name.str.replace(' ', '-') + '_' + dfarray[i].second_name.str.replace(' ', '-')
    
    predictive = True
    return dfarray

def main():
    download = True;
    if os.path.exists("tmp.txt"):
        answer = input("Cache file found, overrite (y/n)?").lower()

    if answer != "y" and answer != 'yes':
        f = open("tmp.txt", encoding ='UTF-8' , mode ='r')
        data = f.read()
        f.close()
    else:
        data = download()
        f = open("tmp.txt", encoding ='UTF-8' , mode ='w')
        f.write(data)
        f.close()
    

    jstr = get_json(data)
    jdata = json.loads(jstr)
   
    filename = "../FantasySack/20" + str(cur_start) + "-" + str(cur_start +1) + '_opta.txt'
    save.save_season_file(get_season_df(jdata), filename )

    df = get_gw_df(jdata);
    for i in range(1, 39):
        if not df[i].empty:
            filename = ".\\gwxpts\\gw" + str(i) + ".txt"
            save.save_gw_file(df[i], filename, True)

    lines = []
    merged_file = "../FantasySack/20" + str(cur_start) + "-" + str(cur_start +1) + "_gw_opta.txt"
    mf = open(merged_file, encoding= 'UTF-16', mode = "w")
    for i in range(1, 39):
        filename = ".\\gwxpts\\gw" + str(i) + ".txt"
        if os.path.exists(filename):
            f = open(filename, encoding= 'UTF-16', mode = "r")
            lines.extend(f.readlines())
            f.close()
    mf.writelines(lines)
    mf.close()


if __name__ == '__main__':
    main()
  