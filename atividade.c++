#include <iostream>
#include <map>
#include <stack>
#include <string>

struct TransitionKey {
    int  state;
    char input;    // '\0' = epsilon
    char stackTop;
    bool operator<(const TransitionKey& other) const {
        if (state    != other.state)    return state    < other.state;
        if (input    != other.input)    return input    < other.input;
        return stackTop < other.stackTop;
    }
};

struct TransitionValue {
    int         nextState;
    std::string pushString; // posicao 0 = topo resultante; "" = epsilon (so pop)
};

class PDA {
public:
    PDA(char initialStackSymbol, int initialState, int finalState)
        : initialStackSymbol_(initialStackSymbol),
          initialState_(initialState),
          finalState_(finalState) {}

    void setStateName(int state, const std::string& name) {
        stateNames_[state] = name;
    }

    void addTransition(int from, char input, char stackTop,
                       int to, const std::string& push) {
        transitions_[{from, input, stackTop}] = {to, push};
    }

    void addEpsilonTransition(int from, char stackTop,
                              int to, const std::string& push) {
        transitions_[{from, '\0', stackTop}] = {to, push};
    }

    bool simulate(const std::string& input) {
        int              currentState = initialState_;
        std::stack<char> stk;
        stk.push(initialStackSymbol_);
        int step = 0;

        printConfig(step, false, currentState, input, stk);

        for (std::size_t pos = 0; pos < input.size(); ) {
            if (stk.empty()) {
                std::cout << "Rejeicao: pilha vazia com entrada restante.\n";
                return false;
            }

            TransitionKey key{currentState, input[pos], stk.top()};
            auto it = transitions_.find(key);
            if (it == transitions_.end()) {
                std::cout << "Rejeicao: nenhuma transicao para ("
                          << stateNames_.at(currentState) << ", "
                          << input[pos] << ", " << stk.top() << ").\n";
                return false;
            }

            const TransitionValue& tv = it->second;
            stk.pop();
            applyPush(stk, tv.pushString);
            currentState = tv.nextState;
            ++pos;
            ++step;

            printConfig(step, false, currentState, input.substr(pos), stk);

            while (applyEpsilonTransition(currentState, stk)) {
                ++step;
                printConfig(step, true, currentState,
                            pos < input.size() ? input.substr(pos) : "", stk);
            }
        }

        while (applyEpsilonTransition(currentState, stk)) {
            ++step;
            printConfig(step, true, currentState, "", stk);
        }

        bool byFinalState = (currentState == finalState_);
        bool byEmptyStack = stk.empty();

        if (byFinalState && byEmptyStack) {
            std::cout << "Cadeia ACEITA por estado final e pilha vazia.\n";
            return true;
        }
        if (byFinalState) {
            std::cout << "Cadeia ACEITA por estado final (pilha nao vazia).\n";
            return true;
        }
        if (byEmptyStack) {
            std::cout << "Cadeia ACEITA por pilha vazia (estado nao final).\n";
            return true;
        }
        std::cout << "Cadeia REJEITADA. Estado: " << stateNames_.at(currentState)
                  << ", Pilha: " << stackToString(stk) << "\n";
        return false;
    }

private:
    std::map<TransitionKey, TransitionValue> transitions_;
    std::map<int, std::string>               stateNames_;
    char initialStackSymbol_;
    int  initialState_;
    int  finalState_;

    static void applyPush(std::stack<char>& stk, const std::string& pushString) {
        for (int i = static_cast<int>(pushString.size()) - 1; i >= 0; --i)
            stk.push(pushString[static_cast<std::size_t>(i)]);
    }

    bool applyEpsilonTransition(int& state, std::stack<char>& stk) {
        if (stk.empty()) return false;
        auto it = transitions_.find({state, '\0', stk.top()});
        if (it == transitions_.end()) return false;
        const TransitionValue& tv = it->second;
        stk.pop();
        applyPush(stk, tv.pushString);
        state = tv.nextState;
        return true;
    }

    std::string stackToString(std::stack<char> s) const {
        std::string result;
        while (!s.empty()) {
            result += s.top();
            s.pop();
        }
        return result.empty() ? "(vazia)" : result;
    }

    void printConfig(int step, bool isEpsilon, int state,
                     const std::string& remaining,
                     const std::stack<char>& stk) const {
        if (step == 0)
            std::cout << "Config. inicial: ";
        else if (isEpsilon)
            std::cout << "Passo " << step << " [epsilon]: ";
        else
            std::cout << "Passo " << step << ": ";

        std::string stackStr = stackToString(stk);
        std::cout << "(" << stateNames_.at(state)
                  << ", " << remaining
                  << ", " << stackStr << ")\n";
    }
};

int main() {
    PDA pda('Z', 0, 2);
    pda.setStateName(0, "q0");
    pda.setStateName(1, "q1");
    pda.setStateName(2, "q2");

    // δ(q0, a, Z) = (q0, AZ)
    pda.addTransition(0, 'a', 'Z', 0, "AZ");
    // δ(q0, a, A) = (q0, AA)
    pda.addTransition(0, 'a', 'A', 0, "AA");
    // δ(q0, b, A) = (q1, ε)
    pda.addTransition(0, 'b', 'A', 1, "");
    // δ(q1, b, A) = (q1, ε)
    pda.addTransition(1, 'b', 'A', 1, "");
    // δ(q1, ε, Z) = (q2, ε)
    pda.addEpsilonTransition(1, 'Z', 2, "");

    std::cout << "=== Simulador de Automato de Pilha ===\n";
    std::cout << "Linguagem reconhecida: { a^n b^n | n >= 1 }\n\n";

    std::string input;
    while (true) {
        std::cout << "Digite uma cadeia (ou 'sair' para encerrar): ";
        std::cin >> input;
        if (input == "sair") break;
        std::cout << "\n";
        pda.simulate(input);
        std::cout << "\n";
    }

    return 0;
}
