/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"
#include "CustomScoreProvider.h"

namespace Lucene
{
	/// Query that sets document score as a programmatic function of several (sub) scores:
	/// <ol>
	/// <li>the score of its subQuery (any query)
	/// <li>(optional) the score of its ValueSourceQuery (or queries).  For most simple/convenient use cases 
	/// this query is likely to be a {@link FieldScoreQuery}
	/// </ol>
	/// Subclasses can modify the computation by overriding {@link #getCustomScoreProvider}.
	class LPPAPI CustomScoreQuery : public Query
	{
	public:
		/// Create a CustomScoreQuery over input subQuery.
		/// @param subQuery the sub query whose scored is being customed. Must not be null. 
		CustomScoreQuery(QueryPtr subQuery);
		
		/// Create a CustomScoreQuery over input subQuery and a {@link ValueSourceQuery}.
		/// @param subQuery the sub query whose score is being customized. Must not be null.
		/// @param valSrcQuery a value source query whose scores are used in the custom score computation. For 
		/// most simple/convenient use case this would be a {@link FieldScoreQuery}.  This parameter is 
		/// optional - it can be null.
		CustomScoreQuery(QueryPtr subQuery, ValueSourceQueryPtr valSrcQuery);
		
		/// Create a CustomScoreQuery over input subQuery and a {@link ValueSourceQuery}.
		/// @param subQuery the sub query whose score is being customized. Must not be null.
		/// @param valSrcQueries value source queries whose scores are used in the custom score computation. 
		/// For most simple/convenient use case these would be {@link FieldScoreQueries}.  This parameter is 
		/// optional - it can be null or even an empty array.
		CustomScoreQuery(QueryPtr subQuery, Collection<ValueSourceQueryPtr> valSrcQueries);
		
		virtual ~CustomScoreQuery();
	
		LUCENE_CLASS(CustomScoreQuery);
	
	protected:
		QueryPtr subQuery;
		Collection<ValueSourceQueryPtr> valSrcQueries; // never null (empty array if there are no valSrcQueries).
		bool strict; // if true, valueSource part of query does not take part in weights normalization.
	
	public:
		using Query::toString;
		
		virtual QueryPtr rewrite(IndexReaderPtr reader);
		virtual void extractTerms(SetTerm terms);
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
		virtual String toString(const String& field);
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();
		
		/// Compute a custom score by the subQuery score and a number of ValueSourceQuery scores.
		///
		/// Deprecated: Will be removed in Lucene 3.1.
		///
		/// The doc is relative to the current reader, which is unknown to CustomScoreQuery when using per-segment 
		/// search (since Lucene 2.9).
		/// Please override {@link #getCustomScoreProvider} and return a subclass of {@link CustomScoreProvider} 
		/// for the given {@link IndexReader}.
		virtual double customScore(int32_t doc, double subQueryScore, Collection<double> valSrcScores);
		
		/// Compute a custom score by the subQuery score and the ValueSourceQuery score.
		///
		/// Deprecated: Will be removed in Lucene 3.1.
		///
		/// The doc is relative to the current reader, which is unknown to CustomScoreQuery when using per-segment 
		/// search (since Lucene 2.9).  
		/// Please override {@link #getCustomScoreProvider} and return a subclass of {@link CustomScoreProvider} 
		/// for the given {@link IndexReader}.
		virtual double customScore(int32_t doc, double subQueryScore, double valSrcScore);
		
		/// Explain the custom score.
		///
		/// Deprecated: Will be removed in Lucene 3.1.
		///
		/// The doc is relative to the current reader, which is unknown to CustomScoreQuery when using per-segment 
		/// search (since Lucene 2.9).
		/// Please override {@link #getCustomScoreProvider} and return a subclass of {@link CustomScoreProvider} 
		/// for the given {@link IndexReader}.
		virtual ExplanationPtr customExplain(int32_t doc, ExplanationPtr subQueryExpl, Collection<ExplanationPtr> valSrcExpls);
		
		/// Explain the custom score.
		///
		/// Deprecated Will be removed in Lucene 3.1.
		///
		/// The doc is relative to the current reader, which is unknown to CustomScoreQuery when using per-segment 
		/// search (since Lucene 2.9).
		/// Please override {@link #getCustomScoreProvider} and return a subclass of {@link CustomScoreProvider} 
		/// for the given {@link IndexReader}.
		virtual ExplanationPtr customExplain(int32_t doc, ExplanationPtr subQueryExpl, ExplanationPtr valSrcExpl);
		
		virtual WeightPtr createWeight(SearcherPtr searcher);
		
		/// Checks if this is strict custom scoring.  In strict custom scoring, the ValueSource part does not 
		/// participate in weight normalization.  This may be useful when one wants full control over how scores 
		/// are modified, and does not care about normalizing by the ValueSource part.  One particular case where 
		/// this is useful if for testing this query.   
		///
		/// Note: only has effect when the ValueSource part is not null.
		virtual bool isStrict();
		
		/// Set the strict mode of this query. 
		/// @param strict The strict mode to set.
		/// @see #isStrict()
		virtual void setStrict(bool strict);
		
		/// A short name of this query, used in {@link #toString(String)}.
		virtual String name();
	
	protected:
		void ConstructQuery(QueryPtr subQuery, Collection<ValueSourceQueryPtr> valSrcQueries);
		
		/// Returns a {@link CustomScoreProvider} that calculates the custom scores for the given {@link 
		/// IndexReader}.  The default implementation returns a default implementation as specified in 
		/// the docs of {@link CustomScoreProvider}.
		CustomScoreProviderPtr getCustomScoreProvider(IndexReaderPtr reader);
		
		friend class CustomWeight;
		friend class CustomScorer;
	};
	
	// when deprecated methods are removed, do not extend class here, just return new default CustomScoreProvider
	class LPPAPI DefaultCustomScoreProvider : public CustomScoreProvider
	{
	public:
	    DefaultCustomScoreProvider(CustomScoreQueryPtr customQuery, IndexReaderPtr reader);
		virtual ~DefaultCustomScoreProvider();
	
		LUCENE_CLASS(DefaultCustomScoreProvider);
	
	protected:
	    CustomScoreQueryWeakPtr _customQuery;
	
	public:
	    virtual double customScore(int32_t doc, double subQueryScore, Collection<double> valSrcScores);
	    virtual double customScore(int32_t doc, double subQueryScore, double valSrcScore);
	    virtual ExplanationPtr customExplain(int32_t doc, ExplanationPtr subQueryExpl, Collection<ExplanationPtr> valSrcExpls);
	    virtual ExplanationPtr customExplain(int32_t doc, ExplanationPtr subQueryExpl, ExplanationPtr valSrcExpl);
	};
	
	class LPPAPI CustomWeight : public Weight
	{
	public:
		CustomWeight(CustomScoreQueryPtr query, SearcherPtr searcher);
		virtual ~CustomWeight();
		
		LUCENE_CLASS(CustomWeight);
	
	public:
		CustomScoreQueryPtr query;
		SimilarityPtr similarity;
		WeightPtr subQueryWeight;
		Collection<WeightPtr> valSrcWeights;
		bool qStrict;
	
	public:
		virtual QueryPtr getQuery();
		virtual double getValue();
		virtual double sumOfSquaredWeights();
		virtual void normalize(double norm);
		virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
		virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
		virtual bool scoresDocsOutOfOrder();
	
	protected:
		ExplanationPtr doExplain(IndexReaderPtr reader, int32_t doc);
	};
	
	/// A scorer that applies a (callback) function on scores of the subQuery.
	class LPPAPI CustomScorer : public Scorer
	{
	public:
		CustomScorer(SimilarityPtr similarity, IndexReaderPtr reader, CustomWeightPtr weight, ScorerPtr subQueryScorer, Collection<ScorerPtr> valSrcScorers);
		virtual ~CustomScorer();
	
		LUCENE_CLASS(CustomScorer);
	
	protected:
		double qWeight;
		ScorerPtr subQueryScorer;
		Collection<ScorerPtr> valSrcScorers;
		IndexReaderPtr reader;
		CustomScoreProviderPtr provider;
		Collection<double> vScores; // reused in score() to avoid allocating this array for each doc 
		
	public:
		virtual int32_t nextDoc();
		virtual int32_t docID();
		virtual double score();
		virtual int32_t advance(int32_t target);
	};
}
