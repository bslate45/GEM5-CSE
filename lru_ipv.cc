#include "mem/cache/replacement_policies/lru_ipv.hh"

#include <cassert>
#include <memory>

#include "params/LRUIPVRP.hh"


int ipv[] = {0, 0, 1, 0, 3, 0, 1, 2, 1, 0, 5, 1, 0, 0, 1, 11, 13};

LRUIPVRP::LRUIPVRP(const Params *p)
    : BaseReplacementPolicy(p)
{
}

/**
 * Set the pointer position to an invalid position (greater than 15).
 */
void
LRUIPVRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    std::static_pointer_cast<LRUIPVReplData>(replacement_data)->position = 16;
}

/**Push all elements from the positions new insertion point (ipv[replacement_data_position]) 
 * up to the current position (static_pointer_cast<LRUIPVReplData>(replacement_data)->position). 
 */
void
LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    //Variable to find the position that is going to be recieved from the IPV
    int replacement_data_position = std::static_pointer_cast<LRUIPVReplData>(replacement_data)->position;
    int start = replacement_data_position;

    ReplacementData *temp1;
    ReplacementData *temp2;
    ReplacementData *temp3;

    for (int i = ipv[replacement_data_position]; i < replacement_data_position; i++) {

	*temp1 = recency_stack[start];
        *temp2 = recency_stack[i];

	temp3 = temp2;
	temp2 = temp1;
	temp1 = temp3;

    }

    //Reset the pointer to the original ipv postion
    std::static_pointer_cast<LRUIPVReplData>(replacement_data)->position = ipv[replacement_data_position];
}

/**Push all elements from the insertion point (index position 13) 
 * up to the eviction point (index position 15), this will in turn push the MRU element
 * outside of the active range. 
 */
void
LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
   int start = 13;

   ReplacementData *temp1;
   ReplacementData *temp2;
   ReplacementData *temp3;

   for (int i = 13; i < 16; i++) {

	*temp1 = recency_stack[start];
        *temp2 = recency_stack[i++];

	temp3 = temp2;
	temp2 = temp1;
	temp1 = temp3;
	

    }

    std::static_pointer_cast<LRUIPVReplData>(replacement_data)->position = 13;
}

/** 
 * Update victim entry if necessary, in this case if it is greater than the eviction point
 * (index position 15).
 */
ReplaceableEntry*
LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const
{

    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates)
    {
        if (std::static_pointer_cast<LRUIPVReplData>(candidate->replacementData)->position > 15 ) {

            victim = candidate;

        }
    }

    return victim;


}

/**
 * Populates the recency stack with new ReplacementData
 */
std::shared_ptr<ReplacementData>
LRUIPVRP::instantiateEntry()
{
    LRUIPVReplData *ptr = new LRUIPVReplData();
    recency_stack.push_back(*ptr);
    return std::shared_ptr<ReplacementData>(ptr);
}

LRUIPVRP*
LRUIPVRPParams::create()
{
    return new LRUIPVRP(this);
}
