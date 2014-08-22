#include <vector>
#include "BilingualLM.h"
#include "moses/ScoreComponentCollection.h"
#include "moses/Hypothesis.h"
#include "moses/InputPath.h"
#include "moses/Manager.h"

using namespace std;

namespace Moses
{
int BilingualLMState::Compare(const FFState& other) const
{
  const BilingualLMState &otherState = static_cast<const BilingualLMState&>(other);

  if (m_targetLen == otherState.m_targetLen)
    return 0;
  return (m_targetLen < otherState.m_targetLen) ? -1 : +1;
}

////////////////////////////////////////////////////////////////
BilingualLM::BilingualLM(const std::string &line)
  :StatefulFeatureFunction(3, line)
{
  ReadParameters();
}

void BilingualLM::Load(){
  m_neuralLM_shared = new nplm::neuralLM(m_filePath, true);
  //TODO: config option?
  m_neuralLM_shared->set_cache(1000000);

  UTIL_THROW_IF2(m_nGramOrder != m_neuralLM_shared->get_order(),
                 "Wrong order of neuralLM: LM has " << m_neuralLM_shared->get_order() << ", but Moses expects " << m_nGramOrder);
}

void BilingualLM::EvaluateInIsolation(const Phrase &source
                , const TargetPhrase &targetPhrase
                , ScoreComponentCollection &scoreBreakdown
                , ScoreComponentCollection &estimatedFutureScore) const {}

void BilingualLM::EvaluateWithSourceContext(const InputType &input
                                  , const InputPath &inputPath
                                  , const TargetPhrase &targetPhrase
                                  , const StackVec *stackVec
                                  , ScoreComponentCollection &scoreBreakdown
                                  , ScoreComponentCollection *estimatedFutureScore) const
{
  /*
  double value = 0;
  if (target_ngrams > targetPhrase.GetSize()) {
      //We have too small of a phrase.
      return;
  }
  for (size_t i = 0; i < targetPhrase.GetSize() - target_ngrams; ++i) {
    //Get source word indexes

    //Insert n target phrase words.
    std::vector<int> words(target_ngrams + source_ngrams);

    //Taken from NeuralLM wrapper more or less
    for (int j = target_ngrams - 1; j < -1; j--){
      const Word& word = targetPhrase.GetWord(i + j); //Target phrase is actually Phrase
      const Factor* factor = word.GetFactor(0); //Parameter here is m_factorType, hard coded to 0
      const std::string string = factor->GetString().as_string();
      int neuralLM_wordID = m_neuralLM->lookup_word(string);
      words.push_back(neuralLM_wordID); //In the paper it seems to be in reverse order
    }
    //Get source context

    //Get alignment for the word we require
    const AlignmentInfo& alignments = targetPhrase.GetAlignTerm();

    //We are getting word alignment for targetPhrase.GetWord(i + target_ngrams -1) according to the paper.
    //Try to get some alignment, because the word we desire might be unaligned.
    std::set<size_t> last_word_al;
    for (int j = 0; j < targetPhrase.GetSize(); j++){
      //Sometimes our word will not be aligned, so find the nearest aligned word right
      if ((i + target_ngrams -1 +j) < targetPhrase.GetSize()){
        last_word_al = alignments.GetAlignmentsForTarget(i + target_ngrams -1 + j);
        if (!last_word_al.empty()){
          break;
        }
      } else if ((i + target_ngrams -1 +j) > 0) {
        //We couldn't find word on the right, try the left.
        last_word_al = alignments.GetAlignmentsForTarget(i + target_ngrams -1 -j);
        if (!last_word_al.empty()){
          break;
        }

      }
      
    }

    //Assume we have gotten some alignment here. Now we get the source words.
    size_t source_center_index;
    if (last_word_al.size() == 1) {
      //We have only one word aligned
      source_center_index = *last_word_al.begin();
    } else { //We have more than one alignments, take the middle one
      int tempidx = 0; //Temporary index to track where the iterator is.
      for (std::set<size_t>::iterator it = last_word_al.begin(); it != last_word_al.end(); it++){
        if (tempidx == last_word_al.size()/2){
          source_center_index = *(it);
          break;
        }
      }
    }

    //We have found the alignment. Now determine how much to shift by to get the actual source word index.
    const WordsRange& wordsRange = inputPath.GetWordsRange();
    size_t phrase_start_pos = wordsRange.GetStartPos();
    size_t source_word_mid_idx = phrase_start_pos + i + target_ngrams -1; //Account for how far the current word is from the start of the phrase.

    const Sentence& source_sent = static_cast<const Sentence&>(input);
    
    //Define begin and end indexes of the lookup. Cases for even and odd ngrams
    int begin_idx;
    int end_idx;
    if (source_ngrams%2 == 0){
      int begin_idx = source_word_mid_idx - source_ngrams/2 - 1;
      int end_idx = source_word_mid_idx + source_ngrams/2;
    } else {
      int begin_idx = source_word_mid_idx - (source_ngrams - 1)/2;
      int end_idx = source_word_mid_idx + (source_ngrams - 1)/2;
    }

    //Add words to vector
    for (int j = begin_idx; j < end_idx; j++) {
      int neuralLM_wordID;
      if (j < 0) {
        neuralLM_wordID = m_neuralLM->lookup_word(BOS_);
      } else if (j > source_sent.GetSize()) {
        neuralLM_wordID = m_neuralLM->lookup_word(EOS_);
      } else {
        const Word& word = source_sent.GetWord(j);
        const Factor* factor = word.GetFactor(0); //Parameter here is m_factorType, hard coded to 0
        const std::string string = factor->GetString().as_string();
        neuralLM_wordID = m_neuralLM->lookup_word(string);
      }
      words.push_back(neuralLM_wordID);
      
    }

    value += m_neuralLM->lookup_ngram(words);
  }
  scoreBreakdown.PlusEquals(FloorScore(value)); //If the ngrams are > than the target phrase the value added will be zero.
*/
}

FFState* BilingualLM::EvaluateWhenApplied(
  const Hypothesis& cur_hypo,
  const FFState* prev_state,
  ScoreComponentCollection* accumulator) const
{
  double totalScore = 0;
  Manager& manager = cur_hypo.GetManager();
  const Sentence& source_sent = static_cast<const Sentence&>(manager.GetSource());

  const Hypothesis * current = &cur_hypo;

  while (current){
    double value = 0;
    Phrase whole_phrase;
    current->GetOutputPhrase(whole_phrase);
    const TargetPhrase& currTargetPhrase = current->GetCurrTargetPhrase();
    const WordsRange& targetWordRange = current->GetCurrTargetWordsRange(); //This should be which words of whole_phrase the current hypothesis represents.


    totalScore += value;
    current = current->GetPrevHypo();
  }

  // dense scores
  vector<float> newScores(m_numScoreComponents);
  newScores[0] = 1.5;
  newScores[1] = 0.3;
  newScores[2] = 0.4;
  accumulator->PlusEquals(this, newScores);

  // int targetLen = cur_hypo.GetCurrTargetPhrase().GetSize(); // ??? [UG]
  return new BilingualLMState(0);
}

FFState* BilingualLM::EvaluateWhenApplied(
  const ChartHypothesis& /* cur_hypo */,
  int /* featureID - used to index the state in the previous hypotheses */,
  ScoreComponentCollection* accumulator) const
{
  return new BilingualLMState(0);
}

void BilingualLM::SetParameter(const std::string& key, const std::string& value)
{
  if (key == "filepath") {
    m_filePath = value;
  } else if (key == "ngrams") {
    m_nGramOrder = atoi(value.c_str());
  } else {
    StatefulFeatureFunction::SetParameter(key, value);
  }
}

}

