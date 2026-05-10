savedcmd_/home/ubuntu/lab5/int_stack.mod := printf '%s\n'   int_stack.o | awk '!x[$$0]++ { print("/home/ubuntu/lab5/"$$0) }' > /home/ubuntu/lab5/int_stack.mod
