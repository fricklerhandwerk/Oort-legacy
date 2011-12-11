// Copyright 2011 Rich Lane
#ifndef OORT_SIM_SHIP_H_
#define OORT_SIM_SHIP_H_

#include <stdint.h>
#include <memory>
#include "sim/entity.h"

namespace Oort {

class Game;
class AI;
class ShipClass;

class Ship : public Entity {
	public:
	const ShipClass *klass;
	std::unique_ptr<AI> ai;
	uint32_t id;

	Ship(Game *game, const ShipClass *klass, std::shared_ptr<Team> team);
	~Ship();

	virtual void tick();
	void fire(float angle);

	void acc_main(float acc);
	void acc_lateral(float acc);
	void acc_angular(float acc); // rad/s^2

	private:
	float main_acc;
	float lateral_acc;
	float angular_acc;

	void update_forces();
};

}

#endif
