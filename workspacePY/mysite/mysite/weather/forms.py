from django import forms     
from django.contrib.auth import get_user_model #my user model
from django.contrib.auth.forms import UserCreationForm      
#from django.db import User_weather


class MyRegistrationForm(UserCreationForm): 
    email = forms.EmailField(required = True)
    #weather_id = forms.CharField(required=True, help_text='to get in www.openweathermap.com')
    #first_name = forms.CharField(required = False)
    #last_name = forms.CharField(required = False)


    class Meta:
        model = get_user_model() #i refer to my user model, so i get user model!
        fields = ('username', 'email', 'password1', 'password2')        

    def save(self,commit = True):   
        user = super(MyRegistrationForm, self).save() #he look my model,thanks to get_user_model() and commit with my data form
        return user
    
    

class SignUpWeatherForm(forms.Form):
    
    #api_key_weather = forms.CharField(required = True,help_text='Necessary, to get in www.openweathermap.com')
    weather_id = forms.CharField(required=True, help_text='Necessary')
    name = forms.CharField(required=True, help_text='Necessary')
    latitude = forms.FloatField(required=True, help_text='Necessary')
    longitude = forms.FloatField(required=True, help_text='Necessary')
    #ico= forms.FileField(required=True) 

  
    def __init__(self,*args,**kwargs):
        self.request = kwargs.pop('request', None)
        super(SignUpWeatherForm,self).__init__(*args,**kwargs)
            

        # Set choices from argument.
   
