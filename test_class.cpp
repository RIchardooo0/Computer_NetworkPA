#include <iostream>
#include <string>

using namespace std;

class SocketObj{
    public:
        string IP;
        string PORT;

        void printIP(){
            cout << "My IP is:\t" << IP << endl;
        }

        void printPORT(){
            cout << "My PORT is:\t" << PORT << endl;
        }

        bool operator<(const SocketObj &other){
            return atoi(this->PORT.c_str()) < atoi(other.PORT.c_str());
        }

        void setIP(string ip){
            this->IP = ip;
        }
        void setPORT(string port){
            this->PORT = PORT;
        }

        SocketObj(string ip, string port){
            this->IP = ip;
            this->PORT = port;
        }

};

int main(){
    SocketObj s1("127.0.0.1", "9999");
    s1.printIP();
    s1.printPORT();

//    if(s1 < s2){
//        cout << "works well" << endl;
//    }
}
