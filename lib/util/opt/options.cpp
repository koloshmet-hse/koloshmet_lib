#include "options.h"

#include <filesystem>

TOptions::TOptions(int argc, char* argv[], std::initializer_list<TCommand> lst)
    : Command{}
    , Executable{GetZeroArg(argc, argv)}
{
    if (argc > 0) {
        std::ostringstream stream;
        std::filesystem::path path{argv[0]};
        stream << "Usage\n";
        stream << '\t' << path.filename().string() << " <command>\n";
        stream << "Commands\n";

        std::size_t alignSize = 0;
        for (auto&& command : lst) {
            alignSize = std::max(alignSize, command.GetName().size());
            if (argc > 1 && command.GetName() == argv[1]) {
                Command = command;
                Command.Init(argc - 2, argv + 2);
                return;
            }
        }
        stream << std::left;
        for (auto&& command : lst) {
            stream << '\t' << std::setw(alignSize);
            stream << command.GetName();
            stream << "  " << command.GetDescription() << '\n';
            if (!command.GetDescription().empty()) {
                stream << '\n';
            }
        }
        std::cerr << stream.str();
        std::exit(1);
    } else {
        throw TException{"Irrelevant argc"};
    }
}

std::size_t TOptions::Size() const {
    return Command.Size();
}

std::string_view TOptions::GetCommand() const {
    return Command.GetName();
}

std::string_view TOptions::GetZeroArg(int argc, char* argv[]) {
    if (argc > 0) {
        return argv[0];
    }
    throw TException{"Irrelevant argc"};
}

const std::filesystem::path& TOptions::GetExecutable() const {
    return Executable;
}
