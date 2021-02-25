try:
    import configparser
except ImportError:
    import ConfigParser as configparser

Import("env")

config = configparser.ConfigParser()
config.read("platformio.ini")
host = config.get("env_custom_target", "custom_ping_host")

def mytarget_callback(*args, **kwargs):
    print("Hello PlatformIO!")
    env.Execute("ping " + host)


env.AlwaysBuild(env.Alias("ping", None, mytarget_callback))