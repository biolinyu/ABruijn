//(c) 2016 by Authors
//This file is a part of ABruijn program.
//Released under the BSD license (see LICENSE file)

#include "general_polisher.h"
#include "alignment.h"

void GeneralPolisher::polishBubble(Bubble& bubble) const
{
	auto optimize = [this] (const std::string& candidate,
							const std::vector<std::string>& branches,
							std::vector<StepInfo>& polishSteps)
	{
		std::string prevCandidate = candidate;
		Alignment align(branches.size(), _subsMatrix);
		while(true)
		{
			StepInfo rec = this->makeStep(prevCandidate, branches, align);
			polishSteps.push_back(rec);
			if (prevCandidate == rec.sequence) return prevCandidate;
			prevCandidate = rec.sequence;
		}
	};

	//first, select closest X branches (by length) and polish with them
	const int PRE_POLISH = 5;
	std::string prePolished = bubble.candidate;
	if (bubble.branches.size() > PRE_POLISH * 2)
	{
		std::sort(bubble.branches.begin(), bubble.branches.end(),
				  [](const std::string& s1, const std::string& s2)
				     {return s1.length() < s2.length();});
		size_t left = bubble.branches.size() / 2 - PRE_POLISH / 2;
		size_t right = left + PRE_POLISH;
		std::vector<std::string> reducedSet(bubble.branches.begin() + left,
											bubble.branches.begin() + right);
		prePolished = optimize(prePolished, reducedSet, 
							   bubble.polishSteps);
	}
	
	//then, polish with all branches
	bubble.candidate = optimize(prePolished, bubble.branches, 
								bubble.polishSteps);
	
}

StepInfo GeneralPolisher::makeStep(const std::string& candidate, 
				   				   const std::vector<std::string>& branches,
								   Alignment& align) const
{
	static char alphabet[] = {'A', 'C', 'G', 'T'};
	StepInfo stepResult;
	
	//Alignment
	double score = 0;
	for (size_t i = 0; i < branches.size(); ++i) 
	{
		score += align.globalAlignment(candidate, branches[i], i);
	}
	stepResult.score = score;
	stepResult.sequence = candidate;

	//Deletion
	bool improvement = false;
	for (size_t pos = 0; pos < candidate.size(); ++pos) 
	{
		double score = 0;
		for (size_t i = 0; i < branches.size(); i++) 
		{
			score += align.addDeletion(i, pos + 1);
		}

		if (score > stepResult.score) 
		{
			stepResult.score = score;
			stepResult.sequence = candidate;
			stepResult.sequence.erase(pos, 1);
			improvement = true;
		}
	}
	if (improvement) return stepResult;

	//Insertion
	for (size_t pos = 0; pos < candidate.size() + 1; ++pos) 
	{
		for (char letter : alphabet)
		{
			double score = 0;
			for (size_t i = 0; i < branches.size(); i++) 
			{
				score += align.addInsertion(i, pos + 1, letter, 
											branches[i]);		
			}

			if (score > stepResult.score) 
			{
				stepResult.score = score;
				stepResult.sequence = candidate;
				stepResult.sequence.insert(pos, 1, letter);
				improvement = true;
			}
		}
	}	
	if (improvement) return stepResult;

	//Substitution
	for (size_t pos = 0; pos < candidate.size(); ++pos) 
	{
		for (char letter : alphabet)
		{
			if (letter == candidate[pos]) continue;

			double score = 0;
			for (size_t i = 0; i < branches.size(); i++) 
			{
				score += align.addSubstitution(i, pos + 1, letter, 
											   branches[i]);
			}

			if (score > stepResult.score) 
			{
				stepResult.score = score;
				stepResult.sequence = candidate;
				stepResult.sequence[pos] = letter;
			}
		}
	}

	return stepResult;
}
