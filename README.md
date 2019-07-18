Simple Socket Programming Task

Implements two basic commands
- 'listall'
- 'send' that takes one argument

For handling multiple clients, we use select() that allows to monitor multiple file descriptors, waiting until one of the file descriptors become active. Select works like an interrupt handler, which gets activated as soon as any file descriptor sends any data.
