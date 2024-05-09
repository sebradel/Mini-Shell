# Mini-Shell
C program that mimicks a shell, with a few basic commands

Supported commands:
  -exit (exits shell)
  -cd [PATH] (changes directory to specified path, or to /home/ if no additional argument. Also updates PWD variable)
  -env (prints all environment variables and their values)
  -setenv VARIABLE VALUE (sets the specified environment variable to the specified value)
  -unsetenv VARIABLE (removes the specified environment variable from the list of environment variables)
  -history (lists the last 500 commands that have been entered)

  INSTRUCTIONS:
    Type 'make' in terminal to compile the C source code, then run by entering ./bsh
  
