/* 
 * File:   Game.cpp
 * Author: cujo
 * 
 * Created on March 4, 2015, 11:56 PM
 */

#include "Game.h"
#include "MineManager.h"
#include "RandomNumber.h"


// Mainly used for debug purposes at the moment.
#include <iostream>

void Game::Start()
{
    if (_gameState != Uninitialized)
        return;
    
    _mainWindow.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32), "Mines Prototype - ZiphTech");
	_mainWindow.setFramerateLimit(120);

    // More init code here.
    
    debugBox.setOrigin(debugBox.GetWidth() / 2, debugBox.GetHeight() / 2);
	debugBox.setPosition(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2);
    
	InitPlayers(m_playerCount);
	

    // DBG: Switch this to start directly into a certain menu in the game.
    // Default: _gameState = Game::ShowSplash;
    _gameState = Game::ShowSplash;
    

	/*sf::Clock c;*/
    // Our entire game loop enters here.
    while (!IsExiting())
    {
		/*sf::Time t = c.getElapsedTime();*/
        GameLoop();
		/*sf::Time t2 = c.getElapsedTime();

		float fps = sf::seconds(1).asSeconds() / (t2.asSeconds() - t.asSeconds());
		std::cout << "FPS: " << fps << '\n';*/
    }
    
    _mainWindow.close();
}

bool Game::IsExiting()
{
    if (_gameState == Game::Exiting)
        return true;
    else
        return false;
}

sf::RenderWindow& Game::GetWindow()
{
    return _mainWindow;
}

DebugBox& Game::GetDebugBox()
{
    return debugBox;
}

//const sf::Event& Game::GetInput()
//{
//    sf::Event e;
//    _mainWindow.pollEvent(e);
//    return e;
//}

void Game::GameLoop()
{
    sf::Event currentEvent;
    
    // START BUG NOTES
    // This is a weird temporary fix for the 
    // collision/game closing bug.
    /*
     * What happens is, when the GameLoop starts, the currentEvent
     * object immediately starts with sf::Event::Closing ONLY IF
     * the collision detection is in the loop.
     * 
     * If you were to comment out the line with the CollisionCheck
     * method you will not have this issue. I have no idea why this
     * happens but I will look into it, until then this is a temporary
     * fix.
     */
    currentEvent.type = sf::Event::GainedFocus;
    // END BUG NOTES
    
    _mainWindow.pollEvent(currentEvent);
    
    switch(_gameState)
    {
        case Game::ShowingMenu:
        {
            ShowMenu();
			InitMines(m_mineCount);
            break;
        }
        case Game::ShowSplash:
        {
            ShowSplashScreen();
            break;
        }
        case Game::Playing:
        {
            _mainWindow.clear(sf::Color(0, 0, 0));
            
            // Update here
			_playerManager.UpdateAll();
            
            
            for (m_it = _mineManager.GetMapObject().begin(); m_it != _mineManager.GetMapObject().end(); ++m_it)
            {
				m_mineCollision = _playerManager.CollisionCheck(m_it->second);
                if (m_mineCollision)
                {
                    _mineManager.Remove(m_it->first);
					break;
                }
            }
            
			m_goalCollision = _playerManager.CollisionCheck(g);
            if (m_goalCollision)
            {
                Game::_gameState = Exiting;
            }
            
            
            // Draw here
			_playerManager.DrawAll(_mainWindow);
            g->Draw(_mainWindow);
            _mineManager.DrawAll(_mainWindow, m_visibleMines);
            
            
            _mainWindow.draw(debugBox);
            
            _mainWindow.display();
            
            // THIS IS BUGGED IF WE DO COLLISION CHECKING
            // I HAVE NO IDEA WHY!!!!
            if (currentEvent.type == sf::Event::Closed)
            {
                _gameState = Game::Exiting;
            }
            
            if (currentEvent.type == sf::Event::KeyPressed)
            {
                if (currentEvent.key.code == sf::Keyboard::Escape)
                {
                    // This is buggy too.
                    ShowMenu();
                }
                //if (currentEvent.key.code == sf::Keyboard::F)
                //{
                //    if (_selectedMine == 0)
                //        m_mineItr = _mineManager.GetMapObject().begin();
                //    
                //    ++m_mineItr;
                //    
                //    if (m_mineItr != _mineManager.GetMapObject().end())
                //    {
                //        _selectedMine = m_mineItr->first;
                //    }
                //    else
                //    {
                //        m_mineItr = _mineManager.GetMapObject().begin();
                //        _selectedMine = m_mineItr->first;
                //    }
                //}
                if (currentEvent.key.code == sf::Keyboard::F5)
                {
                    m_visibleMines = !m_visibleMines;
                }
            }
            
            //if (currentEvent.type == sf::Event::MouseButtonPressed)
            //{
            //    if (currentEvent.mouseButton.button == sf::Mouse::Left)
            //    {
            //        //mine.SetPosition(sf::Mouse::getPosition(_mainWindow).x, sf::Mouse::getPosition(_mainWindow).y);
            //        _mineManager.Get(Game::_selectedMine)->SetPosition(sf::Mouse::getPosition(_mainWindow).x, sf::Mouse::getPosition(_mainWindow).y);
            //    }
            //}
            break;
        }
		case Game::HostPlayer:
		{	
			_mainWindow.close();
			m_host.Hosting();
			break;
		}
		case Game::ClientPlayer:
		{
			NetworkClient m_client;
			PlayerManager m_PlayerManager;
			m_client.SetPlayerManager(m_PlayerManager);

			std::cout << "[CLIENT] Init Client." << '\n';
			
			m_client.player.id = 0;
			m_client.player.name = "Prototype Tester Client 0";
			m_client.player.ping = 0;
			m_client.player.color = "Green";
			m_client.player.isReady = true;
			m_client.player.isWinner = false;
			m_client.player.xPosition = 512;
			m_client.player.yPosition = 384;

			m_client.SetPlayerData(m_client.player);

			
			sf::Thread clientSendData(&NetworkClient::SendPacketData, &m_client);
			sf::Thread clientGetData(&NetworkClient::GetPacketData, &m_client);
			

			
			if (m_client.ConnectToHost(m_client.player))
			{
				std::cout << "[CLIENT] We are connected to Host!" << '\n';

				Player* p = new Player();
				p->SetColor(5);
				p->SetIsClient(true);
				p->SetClientData(m_client.player);
				m_PlayerManager.Add(0, p);
				m_client.SetPlayerManager(m_PlayerManager);

				sf::Event e;
				e.type = sf::Event::GainedFocus;

				clientSendData.launch();
				clientGetData.launch();

				while (e.type != sf::Event::Closed)
				{
					if (m_client.IsNewClient())
					{
						Player* player = new Player();
						player->SetIsClient(true);
						player->SetClientData(m_client.GetPlayerPacketData());
						m_PlayerManager.Add(m_client.GetPlayerPacketData().id, player);
						m_client.SetPlayerManager(m_PlayerManager);
						m_client.SetIsNewClient(false);
					}

					m_client.SetPlayerData(p->GetClientData());

					_mainWindow.pollEvent(e);
					_mainWindow.clear(sf::Color(0, 0, 0));

					m_PlayerManager.UpdateAll();
					m_PlayerManager.DrawAll(_mainWindow);
					_mainWindow.display();

					if (e.type == sf::Event::KeyPressed)
					{
						if (e.key.code == sf::Keyboard::Escape)
						{
							_gameState = Game::Exiting;
							e.type = sf::Event::Closed;
						}
					}

					//m_client.DisplayPacketTraffic();
				}
			}
			else
			{
				std::cout << "[CLIENT] Connection to Host failed." << '\n';
				_gameState = Game::Exiting;
			}

			_gameState = Game::Exiting;
			break;
		}
    }
}

void Game::ShowSplashScreen()
{
    SplashScreen ss;
    ss.Show(_mainWindow);
    _gameState = Game::ShowingMenu;
}

void Game::ShowMenu()
{
    MainMenu menu;
    MainMenu::MenuResult r = menu.Show(_mainWindow);
    switch(r)
    {
        case MainMenu::Exit:
            _gameState = Game::Exiting;
            break;
        case MainMenu::Easy:
            _gameState = Game::Playing;
            m_mineCount = 50;
            break;
        case MainMenu::Medium:
            _gameState = Game::Playing;
            m_mineCount = 150;
            break;
        case MainMenu::Hard:
            _gameState = Game::Playing;
            m_mineCount = 250;
            break;
		case MainMenu::Host:
			_gameState = Game::HostPlayer;
			break;
		case MainMenu::Client:
			_gameState = Game::ClientPlayer;
			break;
    }
}

void Game::InitMines(int count)
{
    int x;
    int y;
    
    for (int i = 0; i < count; i++)
    {
        LandMine* mine = new LandMine();
        
        x = randNum.NewNumber(Game::debugBox.GetBounds().x / 2, Game::debugBox.GetBounds().x);
        y = randNum.NewNumber(Game::debugBox.GetBounds().y / 2, Game::debugBox.GetBounds().y);
        
        mine->SetPosition(x, y);
        
        _mineManager.Add(i, mine);
    }
}

void Game::InitPlayers(int count)
{
	int x = 20;
	int y = 20;

	for (int i = 0; i < count; i++)
	{
		Player* player = new Player();

		/*x = randNum.NewNumber(0, SCREEN_WIDTH);
		y = randNum.NewNumber(0, SCREEN_HEIGHT);*/


		//std::cout << "X RAND: " << x << " Y RAND: " << y << '\n';

		player->SetPosition(x, y);

		_playerManager.Add(i, player);
		x = SCREEN_WIDTH - 20;
		y = SCREEN_HEIGHT - 20;
	}
}


Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;
//Player Game::p;
PlayerManager Game::_playerManager;
MineManager Game::_mineManager;
RandomNumber Game::randNum;
Goal* Game::g = new Goal();
DebugBox Game::debugBox;
int Game::_selectedMine = 0;
bool Game::m_visibleMines = false;
int Game::m_mineCount = 0;
bool Game::m_mineCollision = false;
bool Game::m_goalCollision = false;
int Game::m_playerCount = 1;
std::map<int, LandMine*>::iterator Game::m_it;
//std::map<int, LandMine*>::iterator Game::m_mineItr;
std::map<int, VisibleGameObject*>::iterator Game::m_assetItr;

NetworkHost Game::m_host;
