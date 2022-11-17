from distutils.core import setup, Extension

def main():
    setup(name="H3pwm",
          version="0.1",
          description="Python PWM interface for Allwinner H3 based computer",
          author="Jasper Vos",
          author_email="me@jaspervos.com",
          ext_modules=[Extension("h3pwm", ["python_h3_pwm.c"])])

if __name__ == "__main__":
    main()