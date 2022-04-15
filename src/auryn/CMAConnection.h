/* 
* Copyright 2014-2018 Friedemann Zenke
*
* This file is part of Auryn, a simulation package for plastic
* spiking neural networks.
* 
* Auryn is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Auryn is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Auryn.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CMACONNECTION_H_
#define CMACONNECTION_H_

#include "auryn_definitions.h"
#include "AurynVector.h"
#include "DuplexConnection.h"
#include "Trace.h"
#include "LinearTrace.h"
#include "SpikeDelay.h"


namespace auryn {


/*! \brief CMA alpha beta kappa gamma thingy
 *
 * This class hopefully correctly implements the thing I need before passing to CMA
 * that is the learning rule with 4 parameters wala
 */
class CMAConnection : public DuplexConnection
{

private:
	void init(AurynFloat alpha, AurynFloat beta, AurynFloat kappa, AurynFloat gamma, AurynFloat tau_pre, AurynFloat tau_post, AurynFloat maxweight);

protected:

	AurynDouble hom_fudge;

	Trace * tr_pre;
	Trace * tr_post;

	void propagate_forward();
	void propagate_backward();

	AurynWeight on_pre(NeuronID post);
	AurynWeight on_post(NeuronID pre);

public:
	// AurynFloat A; /*!< Amplitude of post-pre part of the STDP window */
	// AurynFloat B; /*!< Amplitude of pre-post part of the STDP window */
	// I think the problem of alpha, beta .. not working is that im using same name
	// in global public variable and function name.. so im going to make the global
	// ones all caps.
	AurynFloat ALPHA;
	AurynFloat BETA;
	AurynFloat KAPPA;
	AurynFloat GAMMA;

	bool stdp_active;

	// removing some overloadings that I will never really use ..
	CMAConnection(SpikingGroup * source, NeuronGroup * destination, 
			AurynWeight weight, AurynFloat sparseness=0.05, 
			AurynFloat alpha=0,
			AurynFloat beta=0,
			AurynFloat kappa=0,
			AurynFloat gamma=0, 
			AurynFloat tau_pre=20e-3,
			AurynFloat tau_post=20e-3,
			AurynFloat maxweight=1. , 
			TransmitterType transmitter=GLUT,
			string name = "CMAConnection");


	virtual ~CMAConnection();
	virtual void finalize();
	void free();

	virtual void propagate();
	virtual void evolve();

};

}

#endif /*CMACONNECTION_H_*/
