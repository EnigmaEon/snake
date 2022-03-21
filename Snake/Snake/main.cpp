#include <SFML/Graphics.hpp>
#include <iostream>

const int WIDTH = 800, HEIGHT = 800;

bool intersect(sf::CircleShape& a, sf::CircleShape& b) {
	sf::Vector2f d = a.getPosition() - b.getPosition();
	return sqrt(d.x * d.x + d.y * d.y) <= a.getRadius() + b.getRadius();
}

class Feed {
private:
	sf::CircleShape shape;
	float radius = 12;
	float minRadius = 10, maxRadius = 15;
	float radiusDelta = 0.1f;
public:
	Feed(int x, int y) {
		shape.setRadius(radius);
		shape.setPointCount(20);
		shape.setPosition(x, y);
		shape.setOrigin(radius, radius);
		shape.setFillColor(sf::Color::Red);
	}
	sf::Vector2f getPosition() {
		return shape.getPosition();
	}
	sf::CircleShape& getShape() {
		return shape;
	}
	int getRadius(){
		return radius;
	}
	void animate() {
		shape.setRadius(shape.getRadius() + radiusDelta);
		shape.setOrigin(shape.getRadius(), shape.getRadius());
		if (minRadius >= shape.getRadius() || shape.getRadius() >= maxRadius)
			radiusDelta *= -1;
	}
	void update(sf::RenderWindow& win) {
		win.draw(shape);
	}
};

class Snake {
private:
	sf::Vector2f position, vector;
	sf::Text* scoreText;
	std::vector<sf::CircleShape> body;
	float speed, rotateRadius, rotateAngle, segmentRadius;
	int score = 0;
	sf::Vector2f normilizePosition(sf::Vector2f pos) {
		pos.x += WIDTH;
		while (pos.x >= WIDTH) pos.x -= WIDTH;
		pos.y += HEIGHT;
		while (pos.y >= HEIGHT) pos.y -= HEIGHT;
		return pos;
	}
	void moveBody() {
		for (int i = segmentsCount-1; i >= 1; i--) {
			body[i].setPosition(normilizePosition(body[i-1].getPosition()));
		}
		position = normilizePosition(position);
		body[0].setPosition(position);
	}
	void recalcFeatures() {
		speed = std::max(2.f, score / 10.f);
		rotateRadius = std::max(20, score);
		rotateAngle = speed / rotateRadius;
		segmentRadius = std::max(10.f, score / 3.5f);
		for (int i = 0; i < segmentsCount; i++) {
			body[i].setRadius(segmentRadius);
			body[i].setOrigin(segmentRadius, segmentRadius);
		}
	}
public:
	int segmentsCount = 0;
	Snake(sf::Text* scoreTxt) {
		scoreText = scoreTxt;
		
		restart();

		position = sf::Vector2f(WIDTH / 2, HEIGHT / 2);
		vector = sf::Vector2f(0, -1);
		
		addSegment();
	}
	void addSegment() {
		body.push_back(sf::CircleShape(segmentRadius, 20));

		if (!segmentsCount)
			body[segmentsCount].setPosition(position);
		else
			body[segmentsCount].setPosition(body[segmentsCount - 1].getPosition());

		body[segmentsCount].setOrigin(segmentRadius, segmentRadius);
		
		if (segmentsCount % 2 == 0)
			body[segmentsCount].setFillColor(sf::Color::White);
		else
			body[segmentsCount].setFillColor(sf::Color::Red);

		segmentsCount++;
	}
	void addPoint() {
		score++;
		recalcFeatures();
		scoreText->setString(std::to_string(score));
	}
	bool checkForEating(Feed& ball) {
		return intersect(ball.getShape(), body[0]);
	}
	bool checkSelfCollision() {
		for (int i = 13; i < segmentsCount; i++)
			if (intersect(body[0], body[i]))
				return 1;
		return 0;
	}
	void restart() {
		score = -1;
		addPoint();
		recalcFeatures();
		while (segmentsCount > 1) {
			body.pop_back();
			segmentsCount--;
		}
	}
	void move() {
		position += vector * speed;
		moveBody();
	}
	void moveRight() {
		position -= rotateRadius * sf::Vector2f(
			vector.y - vector.x * sin(rotateAngle) - vector.y * cos(rotateAngle),
			-vector.x + vector.x * cos(rotateAngle) - vector.y * sin(rotateAngle)
		);
		vector = {
			vector.x * cos(rotateAngle) - vector.y * sin(rotateAngle),
			vector.x * sin(rotateAngle) + vector.y * cos(rotateAngle)
		};
		moveBody();
	}
	void moveLeft() {
		position += rotateRadius * sf::Vector2f(
			vector.y - vector.x * sin(-rotateAngle) - vector.y * cos(-rotateAngle),
			-vector.x + vector.x * cos(-rotateAngle) - vector.y * sin(-rotateAngle)
		);
		vector = {
			vector.x * cos(-rotateAngle) - vector.y * sin(-rotateAngle),
			vector.x * sin(-rotateAngle) + vector.y * cos(-rotateAngle)
		};
		moveBody();
	}
	void update(sf::RenderWindow& win) {
		for(int i=segmentsCount-1;i>=0;i--)
			win.draw(body[i]);
	}
};

int main() {
	srand(time(0));

	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Snake", sf::Style::Close, settings);
	window.setFramerateLimit(60);

	sf::Font font;
	font.loadFromFile("Whipsmart.ttf");

	sf::Text scoreLabel("Length: ", font, 40);
	sf::Text scoreText("0", font, 40);

	scoreLabel.setPosition(50, 40);
	scoreText.setPosition(175, 40);

	Snake snake(&scoreText);
	Feed ball(WIDTH / 2, HEIGHT / 4);

	bool right = false, left = false;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left)
					left = true;
				else if (event.key.code == sf::Keyboard::Right)
					right = true;
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Left)
					left = false;
				else if (event.key.code == sf::Keyboard::Right)
					right = false;
			}
		}
		window.clear();

		if (left)
			snake.moveLeft();
		else if (right)
			snake.moveRight();
		else
			snake.move();
		
		if (snake.checkForEating(ball)) {
			ball = Feed(20 + rand() % (WIDTH - 40), 20 + rand() % (HEIGHT - 40));
			snake.addPoint();
			snake.addSegment();
		}
		if(snake.checkSelfCollision()){
			snake.restart();
		}

		ball.animate();
		
		snake.update(window);
		ball.update(window);

		window.draw(scoreLabel);
		window.draw(scoreText);

		window.display();
	}
	return 0;
}
