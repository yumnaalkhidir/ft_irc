# FT_IRC

*This project has been created as part of the 42 curriculum by hparveen, yalkhidi.*

## Description

IRC (Internet Relay Chat) is a text-based communication protocol that allows users around the world to communicate through channels and private messages.

ft_irc is a project that aims to recreate a simple IRC server in C++98.  
The server allows multiple clients to connect, join channels, communicate with other users, and manage channel operators and modes.

### Goal of the Project

The goal of this project is to learn:
- networking fundamentals,
- socket programming,
- client/server communication,
- event-driven programming,
- and how IRC servers interact with IRC clients.

---

## Instructions

### Language

This project is written in **C++98**.

### Implemented Commands

- `NICK`
- `PASS`
- `USER`
- `PRIVMSG`
- `NOTICE`
- `JOIN`
- `PART`
- `KICK`
- `INVITE`
- `TOPIC`
- `MODE` (`i`, `l`, `k`, `t`, `o`)
- `WHOIS`

---

## Compilation

```
make
```

### Makefile Rules

```
make        # compile the project
make clean  # remove object files
make fclean # remove object files and executable
make re     # rebuild the project
```

---

## Running the Server

```
./ircserv <port> <password>
```

Example:

```
./ircserv 6667 password
```

---

## Using the Server

During the development of this project, **Irssi** was used as the main IRC client for testing.

Run Irssi:

```
irssi
```

Inside Irssi:

Set a nickname:

```
/set nick <nickname>
```

Connect to the server:

```text
/connect localhost <port> <password>
```

Example:

```text
/connect localhost 6667 password
```
---

## Resources

### IRC Documentation

- RFC 1459  
  https://datatracker.ietf.org/doc/html/rfc1459

- RFC 2812  
  https://datatracker.ietf.org/doc/html/rfc2812

---

## AI Usage

AI tools were used to:
- Help understand IRC concepts and protocol behavior,
- Explain networking and socket programming concepts,
- Assist with understanding how IRC clients such as Irssi operate,
- And help debug and test certain parts of the project.