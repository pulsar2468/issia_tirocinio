import sys; print('%s %s' % (sys.executable or sys.platform, sys.version))
import sys; print('%s %s' % (sys.executable or sys.platform, sys.version))
import sys; print('%s %s' % (sys.executable or sys.platform, sys.version))
python manage.py createsuperuser
import sys; print('%s %s' % (sys.executable or sys.platform, sys.version))
import os; os.environ['DJANGO_SETTINGS_MODULE'] = 'mysite.settings'; import django
if django.VERSION <= (1, 5):
    from django.core import management
    import mysite.settings as settings
    management.setup_environ(settings)
else:
    from django.core.wsgi import get_wsgi_application
    application = get_wsgi_application()
