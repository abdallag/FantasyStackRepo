def save_gw_file(df, filename, predictive):
   f = open(filename, encoding ='UTF-16' , mode ='w')
   
   for row in df.iterrows():
      r = row[1]
      line = str(r.GW) + " " + str(r.player) + " " + str(r.total_points) + " " + str(r.value)
      if predictive:
        line = line + " " + str(r.xpts)
        line = line  + "\n"
        #print(line)
        f.writelines( line)
   f.close()

def save_season_file(df, filename):
    f = open(filename, encoding ='UTF-16' , mode ='w')
    for row in df.iterrows():
      r = row[1]
      line = str(r.name) + " " 
      line = line + r.player
      line = line + " " + str(r.element_type) + " "
      line = line + str(r.total_points) + " " + str(r.value) + " " + str(r.prev_points) + " " + str(r.team_code)  + "\n"
      #print(line)
      f.writelines( line)
    f.close()

