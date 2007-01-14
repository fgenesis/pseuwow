#include "common.h"
#include "DefScript/DefScript.h"
#include "PseuWoW.h"
#include "../dep/src/sqlite/sqlite3.h"
#include "../shared/sqlitewrapped/Database.h"
#include "../shared/sqlitewrapped/Query.h"

#include "Chat.h"

/*
int chatCallBackQuestionTypes(void *arg, int nColumns, char **sValues, char **sNames)
{
	if (nColumns != 2)
	{
		log("Got wrong number of columns for table `talk_types`, columns was: ", nColumns, ", expected 2 columns.");
		return -1;
	}

	ChatTypes[nChatTypes].type = sValues[0];
	ChatTypes[nChatTypes].remove = atoi(sValues[1]);
	nChatTypes++;

	return 0;
}*/


// TODO: Rewrite class (Rewritten from old php code - i didn't knew anything else but very simple select, insert, delete statements when
// the code was originaly written. :P

Chat::Chat(std::string sentence)
{
	// Okay, trying to make a semi-advanced chat AI. :-)
	// First is to find out the type of question, right now there is three types in the database.
	// Opinion - If the player want's the bot to hear if the bot likes or dislikes something
	//			 Example:
	//           Sentence: Do you like mages?
	//			 Answer: Yearh, i love mages!
	//
	// Wazzup - If the player want's to hear what's up :P
	//			 Example:
	//			 Sentence: Wazzup?
	//			 Answer: Don't you have anything better to do?
	//
	// Hi - If the player want's to say hi
	//			 Example:
	//			 Sentece: Hi there!
	//			 Answer: Yo [Name]!
	//
	// ... - more to come

	// How to do it
	// 1. Find the type of sentence
	// 2. Answer the sentence the right way

	result = "";

	Database chatDB("db/chat");

	Query query(chatDB);
	query.get_result("select type, remove from talk_types");

	std::string type, part, resultSentence;
	int remove;

	while (query.fetch_row())
	{
		type = query.getstr();
		remove = query.getval();

		Query query2(chatDB);
		query2.get_result("select part from talk_types_parts where type='" + type + "'");

		printf("Type: %s\n", type.c_str());

		while (query2.fetch_row())
		{
			part = query2.getstr();

			printf("Type: %s, Sentence: %s, Part: %s\n", type.c_str(), sentence.c_str(), part.c_str());

			if (sentence.find(part) != std::string::npos)
			{
				// We got the word type
				if (remove == 1)
				{
					// Remove the sentence part
					resultSentence = sentence.replace(sentence.find(part), part.size(), "");
				}
				else
				{
					resultSentence = sentence;
				}

				int findSign[4];
				findSign[0] = resultSentence.find("?");
				findSign[1] = resultSentence.find("!");
				findSign[2] = resultSentence.find(".");
				findSign[3] = resultSentence.find("?");

				for (int i = 0; i < 4; i++)
				{
					if (findSign[i] >= 0)
					{
						resultSentence = resultSentence.replace(findSign[i], 1, "");
					}
				}


				// TODO: Make trim function (Remove spaces in start and end) - i am to lazy to do it now.
				if (resultSentence[0] == ' ')
				{
					resultSentence.erase(0, 1);
				}

				query2.free_result();
				query.free_result();
				this->Answer(resultSentence, type);
				return;
			}
		}

		query2.free_result();
	}

	query.free_result();
}

std::string Chat::GetResult()
{
	if (result.size() <= 0)
		result = "Err... what are you talking about?"; // Don't know what to answer :P
	return result;
}

void Chat::Answer(std::string sentence, std::string type)
{
	Database chatDB("db/chat");
	std::string extraWhere = "";
	std::string escapedSentence = sentence;

	int findQuote = sentence.find("'");

	if (findQuote >= 0)
	{
		escapedSentence = sentence.replace(findQuote, 1, "''");
	}

	if (type == "Opinion")
	{
		// If the bot doesn't have a opinion, then make a opinion.

		Query numQuery(chatDB);
		numQuery.get_result("select count(*) from talk_answer_remember where question='"+ escapedSentence +"' and type='Opinion'");
		numQuery.fetch_row();

        int nResults = numQuery.getval();
		numQuery.free_result();

		if (nResults <= 0)
		{
			// Doesn't have a opinion, make one.
			Query createOpinionQuery(chatDB);
			uint64 opinion = rand() % 2 + 1; // Random number, 1 or 2

			printf("Making opinion for %s, result: %s\n", sentence.c_str(), opinion == 1 ? "likes" : "dislikes");

			createOpinionQuery.execute("insert into talk_answer_remember (type, question, opinion) values ('Opinion', '" + escapedSentence + "', '" + toString(opinion) + "')");
		}
        
		std::string sentenceOpinion;
		Query opinionQuery(chatDB);
		opinionQuery.get_result("select opinion from talk_answer_remember where question='" + escapedSentence + "' and type='Opinion'");
		opinionQuery.fetch_row();
		sentenceOpinion = opinionQuery.getstr();

		extraWhere = " opinion='"+ sentenceOpinion +"' and";
		opinionQuery.free_result();
	}

	Query answerQuery(chatDB);
	std::string answer;

	std::string opinionQueryStatement = "select answer from talk_answer_text where"+ extraWhere +" type='"+ type +"'";
	answerQuery.get_result(opinionQueryStatement);
	answerQuery.fetch_row();
	answer = answerQuery.getstr();
	std::string replaceString = "%s";
	int find = answer.find(replaceString);
	
	if (find >= 0)
	{
		result = answer.replace(answer.find(replaceString), replaceString.size(), sentence);
	}
	else
	{
		result = answer;
	}

	answerQuery.free_result();
}
