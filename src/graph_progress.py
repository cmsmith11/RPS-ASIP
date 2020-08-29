#!/usr/bin/env python3
import matplotlib.pyplot as plt

x = 0
y = 0
col = 'blue'
name = input()
plt.title(name + '\'s Record Against CPU Over Time')
plt.xlabel('Rounds')
plt.ylabel('Wins - Losses')
plt.scatter(x, y, color=col)
plt.pause(0.0000000000001)
while(True):
    plt.scatter(x, y, color=col)
    plt.pause(0.0000000000001)
    last = y
    try:
        y = int(input())
    except:
        break
    if y > last:
        col = 'green'
    if y < last:
        col = 'red'
    if y == last:
        col = 'blue'
    x += 1
plt.show()
