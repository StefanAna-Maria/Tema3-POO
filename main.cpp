#include <iostream>
#include <vector>
#include <deque>
#include <conio.h>
#include <time.h>
#include <memory>
#include <stdexcept>
#include <windows.h>

using namespace std;

// Clasa pentru exceptii
class GameException : public std::exception
{
private:
    const char* message;

public:
    GameException(const char* msg) : message(msg) {}
    const char* what() const noexcept override
    {
        return message;
    }
};

// Clasa pentru coordonate
template <typename T>
class Coordinate
{
public:
    T x;
    T y;

    Coordinate(T x, T y) : x(x), y(y) {}
};

// Clasa abstracta pentru obiectele din joc
class GameObj
{
public:
    virtual void Move() = 0;
    virtual bool CheckPos(int pos) const = 0;
    virtual std::unique_ptr<GameObj> Clone() const = 0;
    virtual ~GameObj() {}
};

// Clasa pentru jucator
template <typename T>
class Player : public GameObj
{
private:
    Coordinate<T> position;

public:
    Player(int width) : position(width / 2, 0) {}
    Player(const Player& other) : position(other.position) {}

    //getteri pentru pozitia jucatorului
    T getX() const { return position.x; }
    T getY() const { return position.y; }

    void Move() override {}
    bool CheckPos(int pos) const override
    {
        return false;
    }
    std::unique_ptr<GameObj> Clone() const override
    {
        return std::make_unique<Player>(*this);
    }
    //Functii pentru miscarea caracterului(obiect de tip player)
    void moveLeft() { position.x--; }
    void moveRight() { position.x++; }
    void moveUp() { position.y--; }
    void moveDown() { position.y++; }

    Coordinate<T> GetPosition() const { return position; }
};

// Clasa pentru banda pe care se deplaseaza obiectele
class Lane : public GameObj
{
private:
    deque<bool> cars;

public:
    Lane(int width)
    {
        for (int i = 0; i < width; i++)
            cars.push_front(false);
    }

    Lane(const Lane& other) : cars(other.cars) {}

    //Functie pentru generarea obstacolelor(masini) pe benzile jocului
    void Move() override
    {
        if (rand() % 10 == 1)
            cars.push_front(true);
        else
            cars.push_front(false);
        cars.pop_back();
    }

    bool CheckPos(int pos) const override
    {
        return cars[pos];
    }

    std::unique_ptr<GameObj> Clone() const override
    {
        return std::make_unique<Lane>(*this);
    }
};

// Clasa pentru obstacole
class Obstacle : public Lane
{
public:
    Obstacle(int width) : Lane(width) {}

    void Move() override
    {
        Lane::Move();
        if (rand() % 2 == 0)
            Lane::Move();
    }

    std::unique_ptr<GameObj> Clone() const override
    {
        return std::make_unique<Obstacle>(*this);
    }
};

// Functie template libera pentru afisarea coordonatelor
template <typename T>
void PrintCoordinates(const Coordinate<T>& coord)
{
    std::cout << "X: " << coord.x << ", Y: " << coord.y << std::endl;
}

// Design pattern Observer
class PlayerObserver
{
public:
    virtual void OnPlayerMove(const Coordinate<int>& newPosition) = 0;
};

// Clasa jocului
class Game
{
private:
    bool quit;
    int numberOfLanes;
    int width;
    int score;
    std::unique_ptr<Player<int>> player;
    vector<std::unique_ptr<GameObj>> map;
    vector<PlayerObserver*> observers;

public:
    //Se introduc obiecte de tip player, lane si obstacle
    Game(int w = 20, int h = 10) : numberOfLanes(h), width(w), quit(false), player(nullptr), score(0)
    {
        srand(time(0));
        for (int i = 0; i < numberOfLanes; i++)
        {
            if (i % 2 == 0)
                map.push_back(std::make_unique<Lane>(width));
            else
                map.push_back(std::make_unique<Obstacle>(width));
        }

        player = std::make_unique<Player<int>>(width);
    }

    Game(const Game& other) : numberOfLanes(other.numberOfLanes), width(other.width), quit(other.quit), score(other.score)
    {
        player = std::make_unique<Player<int>>(*other.player);
        for (const auto& obj : other.map)
            map.push_back(obj->Clone());
    }

    //Destructor pt clasa game
    ~Game() {}

    //// Functie pentru afisarea starii curente a jocului
    void Draw()
    {
        system("cls");
        for (int i = 0; i < numberOfLanes; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (i == 0 && (j == 0 || j == width - 1))
                    cout << "S";
                if (i == numberOfLanes - 1 && (j == 0 || j == width - 1))
                    cout << "F";
                if (map[i]->CheckPos(j) && i != 0 && i != numberOfLanes - 1)
                    cout << "#";
                else if (player->getX() == j && player->getY() == i)
                    cout << "V";
                else
                    cout << " ";
            }
            cout << endl;
        }
        cout << "Score:" << score << endl;
    }

    // Functie pentru gestionarea input-ului jucatorului
    void Input()
    {
        if (_kbhit())
        {
            char current = _getch();
            if (current == 'a')
                player->moveLeft();
            if (current == 'd')
                player->moveRight();
            if (current == 'w')
                player->moveUp();
            if (current == 's')
                player->moveDown();
            if (current == 'q')
                quit = true;

            // Notificare observatori despre schimbarea pozitiei jucatorului
            for (auto observer : observers)
            {
                observer->OnPlayerMove(player->GetPosition());
            }
        }
    }

    // Functie pentru adaugarea unui observator
    void AddObserver(PlayerObserver* observer)
    {
        observers.push_back(observer);
    }

    // Functie pentru gestionarea logicii jocului
    void Logic()
    {
        for (int i = 0; i < numberOfLanes - 1; i++)
        {
            map[i]->Move();
            //conditie pentru calcularea scorului
            //reprezentat prin numarul de traversari reusite al jucatorului
            if (player->getY() == numberOfLanes - 1)
            {
                score++;
                player->moveUp();
            }
            //conditie care opreste jocul in caz de
            //coleziune intre obiectul tip Player si un obstacol
            if (map[i]->CheckPos(player->getX()) && player->getY() == i)
            {
                quit = true;
            }
        }
    }

    static int highScore;
    static int totalGames;

    // Functie statica pentru obtinerea scorului maxim
    static int GetHighScore()
    {
        return highScore;
    }
    // Functie statica pentru obtinerea numarului total de jocuri jucate
    static int GetTotalGamesPlayed()
    {
        return totalGames;
    }
    // Functia principala de rulare a jocului
    void Run()
    {
        while (!quit)
        {
            Input();
            Draw();
            Logic();
            Sleep(100);
        }
    }
};

int Game::highScore = 0;
int Game::totalGames = 0;

// Design pattern Command
class Command
{
public:
    virtual void Execute(Game& game) = 0;
    virtual ~Command() {}
};

// Comanda pentru a incepe jocul
class StartGameCommand : public Command
{
public:
    void Execute(Game& game) override
    {
        game.Run();
    }
};

// Comanda pentru a iesi din joc
class ExitGameCommand : public Command
{
public:
    void Execute(Game& game) override
    {
        game.Run();
    }
};

// Functie pentru afisarea meniului interactiv
int ShowMenu()
{
    cout << "1. Start Game" << endl;
    cout << "2. Exit Game" << endl;
    cout << "Enter your choice: ";
    int choice;
    cin >> choice;
    return choice;
}

int main()
{
    srand(time(NULL));
    Game::highScore = 0;
    Game::totalGames = 0;

    try
    {
        int choice;
        do
        {
            choice = ShowMenu();
            switch (choice)
            {
            case 1:
            {
                // Start Game
                Game game(30, 5);

                // Adaugare observator pentru jucator
                class PlayerObserverImpl : public PlayerObserver
                {
                public:
                    void OnPlayerMove(const Coordinate<int>& newPosition) override
                    {
                        // Utilizare functiei template libere pentru afisarea coordonatelor
                        PrintCoordinates(newPosition);
                    }
                };

                PlayerObserverImpl observer;
                game.AddObserver(&observer);

                StartGameCommand startCommand;
                startCommand.Execute(game);
                break;
            }
            case 2:
                // Exit Game
                cout << "Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please enter 1 or 2." << endl;
                break;
            }
        } while (choice != 2);
    }
    catch (const std::exception& ex)
    {
        cerr << "Exception:" << ex.what() << endl;
    }

    return 0;
}
