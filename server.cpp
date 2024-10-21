#include<iostream>
#include<queue>
#include<thread>
#include<vector>
#include<mutex>
#include<netinet/in.h>
#include<cstring>
#include<unistd.h>
#include<sstream>
#include<iomanip>
using namespace std;
struct Order{
    int clientSocket;
    double price;
    int quantity;
    bool isBuy;
};
struct BuyCompare{
    bool operator()(const Order&a ,const Order&b)
    {
        return a.price<b.price; //as more willingness to pay more given priority
    }
};

struct SellCompare{
    bool operator()(const Order&a,const Order&b)
    {
        return a.price>b.price;//as seller who want to sell at low price given priority
    }
};

priority_queue<Order,vector<Order>,BuyCompare> buyOrders;
priority_queue<Order,vector<Order>,SellCompare> sellOrders;

mutex orderBookMutex;
double companyProfit=0.0;

void sendMessage(int clientSocket,const string &message)
{
    send(clientSocket,message.c_str(),message.size()+1,0);
}

void matchOrders()
{
    while(!buyOrders.empty()&&!sellOrders.empty())
    {
        Order topBuy=buyOrders.top();
        Order topSell=sellOrders.top();
        if(topBuy.price>=topSell.price)
        {
            int tradeQuant=min(topBuy.quantity,topSell.quantity);
            double tradePrice=topSell.price;

            double extraProfit=(topBuy.price-topSell.price)*tradeQuant;
            companyProfit+=extraProfit;
            string buyMessage="Buy order matched with price: "+to_string(tradePrice)+" for quantity: "+to_string(tradeQuant)+"\n";
            string sellMessage="Sell order matched with price: "+to_string(tradePrice)+" for quantity: "+to_string(tradeQuant)+"\n";
            sendMessage(topBuy.clientSocket,buyMessage);
            sendMessage(topSell.clientSocket,sellMessage);

            topBuy.quantity-=tradeQuant;
            topSell.quantity-=tradeQuant;

            buyOrders.pop();
            sellOrders.pop();
            if(topBuy.quantity>0){buyOrders.push(topBuy);}
            if(topSell.quantity>0){sellOrders.push(topSell);}

            cout<<fixed<<setprecision(2);
            cout<<"Company profit on this trade : $"<<extraProfit<<" Total cash flow in Company $"<<companyProfit<<endl;
        }
        else{
            break;
        }
    }
}

void handleClient(int clientSocket)
{
    char buffer[4096];
    while(true)
    {
        memset(buffer,0,4096);
        int byteRec=recv(clientSocket,buffer,4096,0);
        if(byteRec<=0)
        {
            close(clientSocket);
            break;
        }
        string request(buffer);
        stringstream iss(request);
        string action;
        double price;
        int quantity;
        iss>>action>>price>>quantity;
        Order order;
        order.clientSocket=clientSocket;
        order.price=price;
        order.quantity=quantity;
        lock_guard<mutex>lock(orderBookMutex);
        if(action=="BUY")
        {
            order.isBuy=true;
            buyOrders.push(order);
            sendMessage(clientSocket,"Buy order placed.Please wait founding correct seller for you. \n");
        }
        else if(action=="SELL")
        {
            order.isBuy=false;
            sellOrders.push(order);
            sendMessage(clientSocket,"Sell order placed.Please wait founding correct buyer for you. \n");
        }

        matchOrders();
    }

}

int main()
{
    int serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(serverSocket==-1)
    {
        cerr<<"Can't create socket ! Quitting "<<endl;
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(54000);
    serverAddr.sin_addr.s_addr=INADDR_ANY;

    if(bind(serverSocket,(sockaddr*)&serverAddr,sizeof(serverAddr))==-1)
    {
        cerr<<"Can't bind to IP/port! Quitting"<<endl;
        return -2;
    }

    if(listen(serverSocket,SOMAXCONN)==-1)
    {
        cerr<<"Can't listen ! Quitting"<<endl;
        return -3;
    }

    vector<thread> clientThreads;
    while(true)
    {
        sockaddr_in clientAddr;
        socklen_t clientSize=sizeof(clientAddr);
        int clientSocket=accept(serverSocket,(sockaddr*)&clientAddr,&clientSize);
        if(clientSocket==-1)
        {
            cerr<<"Problem with client connecting!"<<endl;
            continue;
        }
        clientThreads.push_back(thread(handleClient,clientSocket));
    }
        for (auto & thread:clientThreads)
        {
            if(thread.joinable())thread.join();
        }
        close(serverSocket);
        return 0;
        
}
