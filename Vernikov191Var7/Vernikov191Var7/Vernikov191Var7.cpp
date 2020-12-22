#include <iostream>
#include <fstream>
#include <thread>
#include <vector> 
#include <chrono>
#include <mutex>

using namespace std;
using namespace std::this_thread; 
using namespace std::chrono; 

//—труктура, отображающее состо€ние страны
struct State {
    vector <vector<int>> field;
    const int sizeX;
    const int sizeY;
    const int costOfFire;
    const int stateCost;
    const int reoadTime; 
    string nameOfCountry;
    int fireCount = 0;
    int destruction = 0;
};

bool isDestroyed(State& state)
{
    return state.destruction == state.stateCost;
}
bool isCrysis(State& first, State& second)
{
    return first.costOfFire * first.fireCount >= second.stateCost;
}
//ћетоды дл€ проверки на то, могут ли страны продолжать войну. ћен€ет переменную isEnd на false если не могут

int shot(State& state, int x, int y, bool& isEnd) {
    state.destruction += state.field.at(x).at(y);
    int i = state.field.at(x).at(y);
    state.field.at(x).at(y) = 0;
    return i;
}
// —тандартный метод выстрела который считает общий урон на страну

void attack(State& us, State& them, std::mutex& mut, bool& isEnd, int seed)
{
    // ћетод дл€ использовани€ в потоках, совершает обестрелы и уходит на перезар€дку до тех пор, пока не наступит конец войны. (isEnd = true)
    srand(seed);

    while (true)
    {
        if (isEnd)
        {
            break;
        }
        int x = rand() % them.sizeX;
        int y = rand() % them.sizeY;

        while (true)
        {
            if (mut.try_lock() && !isEnd)
            {

                int i = shot(them, x, y, std::ref(isEnd));
                cout << "Artilllery of " << us.nameOfCountry << " fires " << them.nameOfCountry << " and makes " <<
                    i << " of economic destruction" << "(Coordinates are: {" << x << ";" << y << "})\n";

                if (isDestroyed(them))
                {
                    std::cout << them.nameOfCountry << " is comletely destroyed. The war is over \n";
                    isEnd = true;
                }

                if (isCrysis(us, them))
                {
                    std::cout << us.nameOfCountry << " thinks, that the war was too expensive. The war is over \n";
                    isEnd = true;
                }
                mut.unlock();

                break;
            }
            if (isEnd)
            {
                break;
            }
        }
        sleep_for(seconds(us.reoadTime));
    }
}

//ћетод запускает 3 артиллерии на врага
void launchArilllery(State& us, State& them, bool& isEnd, std::mutex& mut)
{
    for (int i = 0; i < 3; i++)
    {
        int seed = rand() % 999999;
        std::thread artillery(attack, std::ref(us), std::ref(them), std::ref(mut), std::ref(isEnd), seed);
        artillery.detach();
    }
}



//–андомайзер на одну страны 
State stateMaker(string nameOfState)
{
    int sizeX = rand() % 21 + 10;
    int sizeY = rand() % 21 + 10;

    int cost = 100;
    int fireCost = rand() % 7 + 3;
    int fireReloadTime = rand() % 2 + 2; 
    int costCounter = 0;
    vector <vector<int>> field;
    field.resize(sizeX);
    for (size_t i = 0; i < sizeX; i++)
    {
        field.at(i).resize(sizeY);
        for (size_t z = 0; z < sizeY; z++)
        {
            int randValue = rand() % 5;
            if (randValue == 0)
            {
                field.at(i).at(z) = cost;
                costCounter += cost;
            }
        }
    }
    cout << "Here is the params of " << nameOfState << " are: \n"
        << "The size of the state : " << sizeX << "x" << sizeY << " Km\n"
        << "The cost of one fire is: " << fireCost 
        << "\nThe total cost of the state is " << costCounter 
        << "\nReload time is " << fireReloadTime << " Sec\n\n";
    State newState = { field, sizeX, sizeY, fireCost, costCounter, fireReloadTime, nameOfState };
    return newState;
}


int main()
{
    //инициализаци€
    State anchuria = stateMaker("Anchuria");
    State taranteria = stateMaker("Taranteria");

    bool isEnd = false;
    std::mutex mutex;

    for (size_t i = 0; i < 5; i++)
    {
        cout << "The war starts in " << 5 - i << " sec\n";
        sleep_for(seconds(1));
    }

    //начало войны
    launchArilllery(std::ref(anchuria), std::ref(taranteria), std::ref(isEnd), std::ref(mutex));
    launchArilllery(std::ref(taranteria), std::ref(anchuria), std::ref(isEnd), std::ref(mutex));

    //¬ывод общей информации пока война идет
    while (!isEnd)
    {
        while (true)
        {
            if (mutex.try_lock())
            {
                cout << "The war is on, Anchuria damage is: " << anchuria.destruction << ", Taranteria damage is " << taranteria.destruction << "\n";
                
                break;
            }
        }
        mutex.unlock();
        sleep_for(seconds(3));
    }
}