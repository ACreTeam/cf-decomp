#include <lib/textinput/tiTextInputBase.h>
#include <stddef.h>

namespace textinput {

void CommandSender::setCommandReceiver(CommandReceiver *receiver) {
    mpCommandReceiver = receiver;
    mpCommandReceiver->addSender(this);
}

void CommandReceiver::onCommand(INPUT_COMMAND command, void *data) {
    for (CommandSender *sender = static_cast<CommandSender *>(
             nw4r::ut::List_GetNext(&mSenderList, NULL));
         sender != NULL; sender = static_cast<CommandSender *>(
                             nw4r::ut::List_GetNext(&mSenderList, sender))) {
        sender->updateFromReceiver((u32)command, data);
    }
}

void CommandReceiver::clearSender() {
    nw4r::ut::List_Init(&mSenderList, offsetof(CommandSender, mLink));
}

} // namespace textinput
