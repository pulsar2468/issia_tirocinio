'''
Created on Apr 6, 2017

@author: nataraja
'''

from django.conf.urls import url
from django.contrib.auth import views as auth_views

from . import views

urlpatterns = [
    url(r'^$', views.index),
    url(r'^weather/data/$', views.real_time),
    url(r'^weather/api1_0/$', views.api1_0),
    url(r'^weather/all_plot/$', views.all_plot),
    url(r'^weather/api1_0/history/$', views.history),
    url(r'^weather/api1_0/city_list/$',views.city_list),
    url(r'^weather/api1_0/getSingleData/$', views.getSingleData),
    url(r'^weather/api1_0/getLastTempData/$',views.getLastTempData),
    url(r'^weather/api1_0/getLastHumyData/$',views.getLastHumyData),
    url(r'^weather/api1_0/getLastPressure/$',views.getLastPressure),
    url(r'^weather/learn/$', views.learn),
    url(r'^weather/joinTelegram/$', views.join_telegram),
    url(r'^weather/login/$', auth_views.login, {'template_name': 'weather/login.html'}, name='login'),    
    url(r'^weather/logout/$', auth_views.logout, {'next_page': '/'}, name='logout'),
    url(r'^weather/signup/$', views.signup, name='signup'),
    url(r'^weather/signup_weather/$', views.signup_weather),
    url(r'^weather/DataFromWs/$', views.DataFromWs),
    url(r'^weather/realtime_from_userStations/$',views.real_time_from_UserStations),
    url(r'^weather/api1_0/getSingleDataRDF/$',views.singleDataRDF),
    url(r'^weather/api1_0/historyRDF/$',views.historyRDF),
    ]