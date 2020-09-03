/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Base client object providing overall control of client IO and rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameclient.h"
#include "gamemessage.h"

#ifndef GAME_DLL
GameClient *g_theGameClient;
#endif

GameMessageDisposition GameClientMessageDispatcher::Translate_Game_Message(const GameMessage *msg)
{
    GameMessage::MessageType type = msg->Get_Type();
    if (type >= GameMessage::MSG_BEGIN_NETWORK_MESSAGES && type <= GameMessage::MSG_END_NETWORK_MESSAGES) {
        return KEEP_MESSAGE;
    }

    if (type == GameMessage::MSG_NEW_GAME || type == GameMessage::MSG_CLEAR_GAME_DATA) {
        return KEEP_MESSAGE;
    }

    return (GameMessageDisposition)(type != GameMessage::MSG_FRAME_TICK);
}
