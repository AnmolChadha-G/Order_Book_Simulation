/*#include<iostream>
#include<queue>
#include<thread>
#include<vector>
#include<mutex>
#include<cstring>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sstream>
#include<iomanip>
using namespace std;
int main()
{
    int clientSocket =socket(AF_INET,SOCK_STREAM,0);
    if(clientSocket==-1)
    {
        cerr<<"Can't create a socket"<<endl;
        return -1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(54000);
    inet_pton(AF_INET,"127.0.0.1",&serverAddr.sin_addr);
    int connectionResult=connect(clientSocket,(sockaddr*)&serverAddr,sizeof(serverAddr));
    if(connectionResult==-1)
    {
        cerr<<"Can't connect to server"<<endl;
        return -2;
    }
    char buffer[4096];
    string userInput;
    while(true)
    {
        cout<<"Enter Order (BUY <PRICE> <QUANTITY>) OR (SELL <PRICE> <QUANTITY>): ";
        getline(cin,userInput);
        int sendResult=send(clientSocket,userInput.c_str(),userInput.size()+1,0);
        if(sendResult==-1)
        {
                cerr<<"Could not send to server "<<endl;
                break;
        }
        memset(buffer,0,4096);
        int bytRec=recv(clientSocket,buffer,4096,0);
        if(bytRec==-1)
        {
            cerr<<"Error getting response from server!"<<endl;
        }
        cout<<"SERVER"<<string(buffer,bytRec)<<endl;
    }
    close(clientSocket);
    return 0;
}*/
#include <iostream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>

using namespace std;

void receiveMessages(int clientSocket) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        if (bytesReceived > 0) {
            cout << "\nSERVER: " << string(buffer, bytesReceived) << endl;
            cout << "Enter Order (BUY <PRICE> <QUANTITY>) OR (SELL <PRICE> <QUANTITY>): ";
            cout.flush();  // Flush the output stream to prevent the prompt from being overwritten
        }
        else if (bytesReceived == 0) {
            cout << "Server disconnected." << endl;
            break;
        }
        else {
            cerr << "Error receiving message from server!" << endl;
            break;
        }
    }
}

int main() {
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Can't create a socket" << endl;
        return -1;
    }

    // Create server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);  // Port number

    // Convert the server's IP address to binary
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Connect to the server
    int connectionResult = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connectionResult == -1) {
        cerr << "Can't connect to server" << endl;
        return -2;
    }

    // Start a thread to handle receiving messages
    thread receiveThread(receiveMessages, clientSocket);

    // Handle sending messages to the server
    string userInput;
    while (true) {
        cout << "Enter Order (BUY <PRICE> <QUANTITY>) OR (SELL <PRICE> <QUANTITY>): ";
        getline(cin, userInput);

        if (userInput.empty()) {
            continue;  // Skip empty input
        }

        // Send the user's input to the server
        int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == -1) {
            cerr << "Could not send to server " << endl;
            break;
        }
    }

    // Wait for the receive thread to finish before closing the socket
    receiveThread.join();

    // Close the socket
    close(clientSocket);

    return 0;
}
