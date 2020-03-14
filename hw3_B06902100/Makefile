CC = g++
OPENCV =  `pkg-config --cflags --libs opencv`

SENDER = sender.cpp
RECEIVER = receiver.cpp
AGENT = agent.c
SEN = sender
REC = receiver
AGE = agent

all: sender receiver agent
  
sender: $(SENDER)
	$(CC) $(SENDER) -o $(SEN)  $(OPENCV) 
receiver: $(RECEIVER)
	$(CC) $(RECEIVER) -o $(REC)  $(OPENCV)
agent:	$(AGENT)
	$(CC) $(AGENT) -o $(AGE)	

.PHONY: clean

clean:
	rm $(CLI) $(SER) $(AGE)
