import numpy as np
import requests
import json
import pandas as pd
import save

def cur_week_data():
  newdf = pd.DataFrame()
  resp = requests.get("https://fantasy.premierleague.com/api/bootstrap-static/#/")
  if resp.status_code != 200:
    print("Error retrieving live data")
  data = json.loads(resp.text)

  gw = -1
  for e in data['events']:
    gw = e['id']
    print(gw)
    if e['is_current']:
      break
  if gw == -1:
    print("No current game week found")
    return

  i = 0
  for p in data['elements']:
    i = i + 1
    row = pd.DataFrame([[ gw , p['first_name'], p['second_name'], p["event_points"], p["now_cost"]]],
                      columns=[ "GW", "first_name", "second_name", "total_points", "value"],
                      index = [i])
    newdf = pd.concat([newdf, row])

  newdf['player'] = newdf.first_name.str.replace(' ', '-') + '_' + newdf.second_name.str.replace(' ', '-')
  return newdf


def main():
    print("Downlaoding live data ...")
    livedf = cur_week_data()

    livegw = -1
    for row in livedf.iterrows():
        livegw = row[1].GW
        break
    print("Live game week = " + str(livegw))

    gwfile = "../../Fantasy-Premier-League/data/2020-21/gws/merged_gw.csv"
    df = pd.read_csv(gwfile)
    df['name'] = df.name.str.replace(' ','-')

    df = df.query('GW != ' + str(livegw))
    df = df[['name','GW', 'value', 'total_points']].rename(columns={'name':'player'}).reset_index()


    df = pd.concat([df, livedf])
   
    save.save_gw_file(df, "2020-21_gw.txt", False)

if __name__ == '__main__':
    main()
