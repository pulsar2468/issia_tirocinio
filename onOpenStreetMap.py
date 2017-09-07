import folium
import sqlite3
import datetime
from bokeh.models import VBox, ColumnDataSource, TableColumn, DataTable, DateFormatter, NumberFormatter, BoxAnnotation, \
    Button, PreText
from bokeh.plotting import figure
import owm_test
from bokeh.resources import CDN
from bokeh.embed import file_html


def visual(name, lon, lat, pressure, temp, humidity, wind_speed, t,wind_degree):
    map_1 = folium.Map(location=[37.57, 13.92], zoom_start=8, tiles='stamenwatercolor')
    feature_group = folium.FeatureGroup("Locations")

    for lat, lng, name, i_temp,time,h,wind,pr,deg in zip(lat, lon, name, temp,t,humidity,wind_speed,pressure,wind_degree):
        if i_temp < 14:
            color = "blue"
        else:
            color = "red"

        html = '<font face="Verdana" size="3" color="red">'  \
               'Ws: %s<br>' \
               'Lat: %s<br>' \
               'Lon: %s<br>' \
               'Temp: %s<br>' \
               'Detection_Time: %s<br>' \
               'Humidity: %.2f<br>' \
               'Pressure: %.2f<br>' \
               'Wind_deg: %.2f<br>' \
               'Wind_speed: %.2f</font>' \
               %(name,lat,lng,i_temp,time,h,pr,deg,wind)
        frame_html = folium.Html(html, script=True, width=250, height=230)
        popup = folium.Popup(frame_html)

        feature_group.add_child(folium.CircleMarker(location=[lat, lng], radius=30, popup=popup, fill_color=color))

    map_1.add_child(feature_group)
    map_1.save("real_timeMap.html")


def real_time(who):

    if who:
        name, lon, lat, pressure, temp, humidity, wind_speed, t, id, wind_deg = owm_test.get_value_from_rectangle()
        visual(name, lon, lat, pressure, temp, humidity, wind_speed, t, wind_deg)
    else:
        name, lon, lat, pressure, temp, humidity, wind_speed, t, wind_deg=owm_test.get_value_from_userStations()
        visual(name, lon, lat, pressure, temp, humidity, wind_speed, t, wind_deg)



def schema(response):
    temp = []
    dT = []
    wind=[]
    hum=[]
    pressure=[]
    wind_deg=[]
    latest_list = []
    conn = sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
    c = conn.cursor()
    sql = 'SELECT City.id,''"%s".name,"%s".detection_time,' \
          'City.lat,City.lon,"%s".temp,"%s".humidity,"%s".wind_speed, "%s".pressure, "%s".wind_deg ' \
          'FROM "%s",City WHERE City.name="%s".name' % (
          response, response, response, response, response, response, response,response,response)
    for row in c.execute(sql):
        latest_list.append(row)
    conn.close()

    for i in range(0, len(latest_list)):
        dT.append(datetime.datetime.strptime(latest_list[i][2], "%Y-%m-%d %H:%M:%S"))
        temp.append(latest_list[i][5])
        hum.append(latest_list[i][6])
        wind.append(latest_list[i][7])
        pressure.append(latest_list[i][8])
        wind_deg.append(latest_list[i][9])

    #Plot 1
    p1 = figure(width=800, height=400, tools='pan,box_zoom,reset',x_axis_type="datetime")
    p1.line(dT,temp)
    p1.title.text = "Temperature"
    p1.xaxis.axis_label = 'Time'
    p1.yaxis.axis_label = 'Value'
    low_box = BoxAnnotation(plot=p1, top=15, fill_alpha=0.4, fill_color='#084594')
    mid_box = BoxAnnotation(plot=p1, bottom=15, top=23, fill_alpha=0.4, fill_color='#FBA40A')
    high_box = BoxAnnotation(plot=p1, bottom=23, fill_alpha=0.5, fill_color='red')
    p1.renderers.extend([low_box,mid_box, high_box])
    p1.toolbar.logo=None




    p2 = figure(width=800, height=300, tools='pan,box_zoom,reset',x_axis_type="datetime")
    p2.line(dT,hum)
    p2.title.text = "Humidity"
    p2.xaxis.axis_label = 'Time'
    p2.yaxis.axis_label = 'Value'
    p2.toolbar.logo=None



    p3 = figure(width=800, height=300, tools='pan,box_zoom,reset', x_axis_type="datetime")
    p3.line(dT, wind)
    p3.title.text = "Wind Speed"
    p3.xaxis.axis_label = 'Time'
    p3.yaxis.axis_label = 'Value'
    p3.toolbar.logo=None



    p4 = figure(width=800, height=300, tools='pan,box_zoom,reset', x_axis_type="datetime")
    p4.multi_line([dT,dT,dT], [temp,hum,wind],line_color=['#0C0786', '#CA4678', '#EFF821'])
    p4.title.text = "All"
    p4.xaxis.axis_label = 'Time'
    p4.yaxis.axis_label = 'Value'
    p4.toolbar.logo=None
    p4.legend.location = "top_left"
    p4.legend.click_policy = "hide"

    p5 = figure(width=800, height=300, tools='pan,box_zoom,reset', x_axis_type="datetime")
    p5.line(dT, pressure)
    p5.title.text = "Pressure"
    p5.xaxis.axis_label = 'Time'
    p5.yaxis.axis_label = 'Value'
    p5.toolbar.logo = None



    #Creation dataTable of history city
    data = dict(
        dates=[i.ctime() for i in dT],
        temperature=temp,
        humidity=hum,
        wind=wind,
        pressure=pressure,
        wind_deg=wind_deg
    )
    source = ColumnDataSource(data)

    columns = [
        TableColumn(field="dates", title="Date", width=300),
        TableColumn(field="temperature", title="Temperature"),
        TableColumn(field="humidity", title="Humidity"),
        TableColumn(field="wind", title="Wind_speed m/s"),
        TableColumn(field="pressure", title="Pressure"),
        TableColumn(field="wind_deg", title="Wind degree")
    ]
    data_table = DataTable(source=source, columns=columns, width=800, height=280)

    #button = Button(label="Download it!", button_type="success",clicks=on_click(latest_list))
    #pre = PreText(text=(json.dumps({'results': [i for i in latest_list]})), width=800, height=1000)

    p=VBox(p1,p2,p3,p4,p5,data_table) #pre
    html=file_html(p,CDN)
    return html

