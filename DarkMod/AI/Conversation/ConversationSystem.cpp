/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision$
 * $Date$
 * $Author$
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id$", init_version);

#include "ConversationSystem.h"

#define CONVERSATION_ENTITYCLASS "atdm:conversation_info"

namespace ai {

void ConversationSystem::Clear()
{
	_conversations.Clear();
	_activeConversations.Clear();
	_dyingConversations.Clear();
}

void ConversationSystem::Init(idMapFile* mapFile)
{
	DM_LOG(LC_CONVERSATION, LT_INFO)LOGSTRING("Searching for difficulty setting on worldspawn.\r");

	if (mapFile->GetNumEntities() <= 0) {
		return; // no entities!
	}

	// Fetch the worldspawn
	for (int i = 0; i < mapFile->GetNumEntities(); i++)
	{
		idMapEntity* mapEnt = mapFile->GetEntity(i);

		idStr className = mapEnt->epairs.GetString("classname");

		if (className == CONVERSATION_ENTITYCLASS)
		{
			// Found an entity, parse the conversation from it
			LoadConversationEntity(mapEnt);
		}
	}

	DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("%d Conversations found in this map.\r", _conversations.Num());
	gameLocal.Printf("ConversationManager: Found %d valid conversations.\n", _conversations.Num());
}

ConversationPtr ConversationSystem::GetConversation(const idStr& name)
{
	// Find the index and pass the call to the overload
	return GetConversation(GetConversationIndex(name));
}

ConversationPtr ConversationSystem::GetConversation(int index)
{
	return (index >= 0 && index < _conversations.Num()) ? _conversations[index] : ConversationPtr();
}

int ConversationSystem::GetConversationIndex(const idStr& name)
{
	for (int i = 0; i < _conversations.Num(); i++)
	{
		if (_conversations[i]->GetName() == name)
		{
			return i;
		}
	}

	return -1;
}

void ConversationSystem::StartConversation(int index)
{
	ConversationPtr conv = GetConversation(index);

	if (conv == NULL)
	{
		gameLocal.Warning("StartConversation: Can't find conversation with index %d\n", index);
		return;
	}

	DM_LOG(LC_CONVERSATION, LT_INFO)LOGSTRING("Trying to start conversation %s.\r", conv->GetName().c_str());

	// Check if the conditions to start the conversations is met
	if (!conv->CheckConditions())
	{
		DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Cannot start conversation %s, conditions are not met.\r", conv->GetName().c_str());
		return;
	}

	// Start the conversation
	conv->Start();

	// Register this conversation for processing
	_activeConversations.AddUnique(index);
}

void ConversationSystem::EndConversation(int index)
{
	ConversationPtr conv = GetConversation(index);

	if (conv == NULL)
	{
		gameLocal.Warning("StartConversation: Can't find conversation with index %d\n", index);
		return;
	}

	// Let the conversation end in a controlled fashion
	conv->End();

	_dyingConversations.AddUnique(index);
}

void ConversationSystem::ProcessConversations()
{
	// Remove the dying conversations first
	for (int i = 0; i < _dyingConversations.Num(); i++)
	{
		// Remove the dying index from the active conversations list
		_activeConversations.Remove(_dyingConversations[i]);
	}

	_dyingConversations.Clear();

	// What remains is a list of active conversations
	for (int i = 0; i < _activeConversations.Num(); i++)
	{
		ConversationPtr conv = GetConversation(_activeConversations[i]);
		assert(conv != NULL);

		// Let the conversation do its job
		if (!conv->Process())
		{
			// Job returned false, terminate this conversation
			DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Terminating conversation %s due to error.\r", conv->GetName().c_str());
			EndConversation(_activeConversations[i]);
			continue;
		}
		
		// Check if the conversation is done
		if (conv->IsDone())
		{
			// Job returned false, terminate this conversation
			DM_LOG(LC_CONVERSATION, LT_INFO)LOGSTRING("Conversation %s finished normally.\r", conv->GetName().c_str());
			EndConversation(_activeConversations[i]);
			continue;
		}
	}
}

void ConversationSystem::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(_conversations.Num());
	for (int i = 0; i < _conversations.Num(); i++)
	{
		_conversations[i]->Save(savefile);
	}

	savefile->WriteInt(_activeConversations.Num());
	for (int i = 0; i < _activeConversations.Num(); i++)
	{
		savefile->WriteInt(_activeConversations[i]);
	}

	savefile->WriteInt(_dyingConversations.Num());
	for (int i = 0; i < _dyingConversations.Num(); i++)
	{
		savefile->WriteInt(_dyingConversations[i]);
	}
}

void ConversationSystem::Restore(idRestoreGame* savefile)
{
	_conversations.Clear();

	int num;
	savefile->ReadInt(num);
	_conversations.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		// Allocate a new conversation and restore it
		_conversations[i] = ConversationPtr(new Conversation);
		_conversations[i]->Restore(savefile);
	}

	_activeConversations.Clear();

	savefile->ReadInt(num);
	_activeConversations.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadInt(_activeConversations[i]);
	}

	_dyingConversations.Clear();

	savefile->ReadInt(num);
	_dyingConversations.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadInt(_dyingConversations[i]);
	}
}

void ConversationSystem::LoadConversationEntity(idMapEntity* entity)
{
	assert(entity != NULL);

	DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Investigating conversation entity %s.\r", entity->epairs.GetString("name"));

	// The conversation index starts with 1, not zero
	for (int i = 1; i < INT_MAX; i++)
	{
		DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Attempting to parse using conversation index %d.\r", i);

		// Attempt to construct a new Conversation object
		ConversationPtr conv(new Conversation(entity->epairs, i));

		if (conv->IsValid())
		{
			// Add the conversation to the list
			_conversations.Append(conv);
		}
		else
		{
			// This loop breaks on the first invalid conversation
			DM_LOG(LC_CONVERSATION, LT_DEBUG)LOGSTRING("Conversation entity %s: found %d valid conversations.\r", entity->epairs.GetString("name"), i-1);
			break;
		}
	}
}

} // namespace ai
