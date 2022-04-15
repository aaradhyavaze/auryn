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

#include "CMAConnection.h"

using namespace auryn;

// start
void CMAConnection::init(AurynFloat alpha, AurynFloat beta, AurynFloat kappa, AurynFloat gamma, AurynFloat tau_pre, AurynFloat tau_post, AurynFloat maxweight)
{
	if (dst->get_post_size() == 0) return;

	auryn::logger->parameter("alpha", alpha);
	auryn::logger->parameter("beta", beta);
	auryn::logger->parameter("kappa", kappa);
	auryn::logger->parameter("gamma", gamma);

	ALPHA = alpha;
	BETA = beta;
	KAPPA = kappa;
	GAMMA = gamma;

	tr_pre = src->get_pre_trace(tau_pre);
	tr_post = src->get_post_trace(tau_post);

	set_min_weight(0.0);
	set_max_weight(maxweight);
	stdp_active = true;
}



void CMAConnection::finalize() {
	DuplexConnection::finalize();
}

void CMAConnection::free()
{
}


CMAConnection::CMAConnection(SpikingGroup * source, NeuronGroup * destination, 
		AurynWeight weight, AurynFloat sparseness, 
		AurynFloat alpha,
		AurynFloat beta,
		AurynFloat kappa,
		AurynFloat gamma, 
		AurynFloat tau_pre,
		AurynFloat tau_post,
		AurynFloat maxweight, 
		TransmitterType transmitter,
		std::string name) 
: DuplexConnection(source, 
		destination, 
		weight, 
		sparseness, 
		transmitter, 
		name)
{
	init(alpha, beta, kappa, gamma, tau_pre, tau_post, maxweight);
	if ( name.empty() )
		set_name("CMAConnection");
}

CMAConnection::~CMAConnection()
{
	if ( dst->get_post_size() > 0 ) 
		free();
}


AurynWeight CMAConnection::on_pre(NeuronID post)
{
	NeuronID translated_spike = dst->global2rank(post); // only to be used for post traces
	AurynDouble dw = ALPHA + KAPPA*tr_post->get(translated_spike);
	return dw;
}

AurynWeight CMAConnection::on_post(NeuronID pre)
{
	AurynDouble dw = BETA + GAMMA*tr_pre->get(pre);
	return dw;
}


void CMAConnection::propagate_forward()
{
	// loop over all spikes
	for (SpikeContainer::const_iterator spike = src->get_spikes()->begin() ; // spike = pre_spike
			spike != src->get_spikes()->end() ; ++spike ) {
		// loop over all postsynaptic partners
		for (const NeuronID * c = w->get_row_begin(*spike) ; 
				c != w->get_row_end(*spike) ; 
				++c ) { // c = post index

			// transmit signal to target at postsynaptic neuron
			AurynWeight * weight = w->get_data_ptr(c); 
			transmit( *c , *weight );

			// handle plasticity
			if ( stdp_active ) {
				// performs weight update
			    *weight += on_pre(*c);

				// clips weights
				if ( *weight > get_max_weight() ) *weight = get_max_weight(); 
				else
			    if ( *weight < get_min_weight() ) *weight = get_min_weight();
			}
		}
	}
}

void CMAConnection::propagate_backward()
{
	if (stdp_active) { 
		SpikeContainer::const_iterator spikes_end = dst->get_spikes_immediate()->end();
		// loop over all spikes
		for (SpikeContainer::const_iterator spike = dst->get_spikes_immediate()->begin() ; // spike = post_spike
				spike != spikes_end ; 
				++spike ) {

			// loop over all presynaptic partners
			for (const NeuronID * c = bkw->get_row_begin(*spike) ; c != bkw->get_row_end(*spike) ; ++c ) {

				#if defined(CODE_ACTIVATE_PREFETCHING_INTRINSICS) && defined(CODE_USE_SIMD_INSTRUCTIONS_EXPLICITLY)
				// prefetches next memory cells to reduce number of last-level cache misses
				_mm_prefetch((const char *)bkw->get_data(c)+2,  _MM_HINT_NTA);
				#endif

				// computes plasticity update
				AurynWeight * weight = bkw->get_data(c); 
				*weight += on_post(*c);

				// clips weights
				if ( *weight > get_max_weight() ) *weight = get_max_weight(); 
				else
			    if ( *weight < get_min_weight() ) *weight = get_min_weight();
			}
		}
	}
}

void CMAConnection::propagate()
{
	propagate_forward();
	propagate_backward();
}

void CMAConnection::evolve()
{
}

