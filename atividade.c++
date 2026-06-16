#include <iostream>
#include <map>
#include <stack>
#include <string>

// Chave da tabela de transicoes: identifica unicamente uma transicao pelo
// estado atual, simbolo lido da entrada e simbolo no topo da pilha.
// '\0' no campo input representa transicao epsilon (sem consumir entrada).
struct TransitionKey {
    int  state;
    char input;    // '\0' = epsilon
    char stackTop;

    // Necessario para usar TransitionKey como chave de std::map,
    // que exige ordenacao total entre as chaves.
    bool operator<(const TransitionKey& other) const {
        if (state    != other.state)    return state    < other.state;
        if (input    != other.input)    return input    < other.input;
        return stackTop < other.stackTop;
    }
};

// Resultado de uma transicao: proximo estado e o que sera empilhado.
// pushString vazia ("") significa epsilon — apenas desempilha o topo,
// sem colocar nada novo na pilha.
// Convencao: o primeiro caractere de pushString fica no topo da pilha.
// Ex.: "AZ" => A fica no topo, Z fica abaixo.
struct TransitionValue {
    int         nextState;
    std::string pushString;
};

class PDA {
public:
    // Inicializa o automato com o simbolo de fundo de pilha,
    // o estado inicial e o unico estado final.
    PDA(char initialStackSymbol, int initialState, int finalState)
        : initialStackSymbol_(initialStackSymbol),
          initialState_(initialState),
          finalState_(finalState) {}

    // Associa um inteiro de estado ao seu nome de exibicao (ex.: 0 -> "q0").
    void setStateName(int state, const std::string& name) {
        stateNames_[state] = name;
    }

    // Registra uma transicao normal: consome um simbolo da entrada.
    void addTransition(int from, char input, char stackTop,
                       int to, const std::string& push) {
        transitions_[{from, input, stackTop}] = {to, push};
    }

    // Registra uma transicao epsilon: nao consome simbolo da entrada.
    // Internamente usa '\0' como sentinel no campo input da chave.
    void addEpsilonTransition(int from, char stackTop,
                              int to, const std::string& push) {
        transitions_[{from, '\0', stackTop}] = {to, push};
    }

    // Executa o DPDA sobre a cadeia de entrada.
    // Imprime cada configuracao (estado, entrada restante, pilha) passo a passo.
    // Retorna true se a cadeia for aceita, false caso contrario.
    bool simulate(const std::string& input) {
        int              currentState = initialState_;
        std::stack<char> stk;
        stk.push(initialStackSymbol_);  // pilha comeca com o simbolo de fundo Z
        int step = 0;

        printConfig(step, false, currentState, input, stk);

        // Percorre cada simbolo da entrada
        for (std::size_t pos = 0; pos < input.size(); ) {

            // Pilha vazia antes de acabar a entrada = rejeicao imediata
            if (stk.empty()) {
                std::cout << "Rejeicao: pilha vazia com entrada restante.\n";
                return false;
            }

            // Busca a transicao para (estado atual, simbolo lido, topo da pilha)
            TransitionKey key{currentState, input[pos], stk.top()};
            auto it = transitions_.find(key);
            if (it == transitions_.end()) {
                std::cout << "Rejeicao: nenhuma transicao para ("
                          << stateNames_.at(currentState) << ", "
                          << input[pos] << ", " << stk.top() << ").\n";
                return false;
            }

            // Aplica a transicao: desempilha o topo e empilha o novo conteudo
            const TransitionValue& tv = it->second;
            stk.pop();
            applyPush(stk, tv.pushString);
            currentState = tv.nextState;
            ++pos;   // consome o simbolo da entrada
            ++step;

            printConfig(step, false, currentState, input.substr(pos), stk);

            // Apos cada transicao normal, verifica se ha transicoes epsilon
            // encadeadas a disparar (sem consumir mais entrada)
            while (applyEpsilonTransition(currentState, stk)) {
                ++step;
                printConfig(step, true, currentState,
                            pos < input.size() ? input.substr(pos) : "", stk);
            }
        }

        // Ao esgotar a entrada, tenta disparar transicoes epsilon restantes
        // (ex.: a transicao final que leva ao estado de aceitacao)
        while (applyEpsilonTransition(currentState, stk)) {
            ++step;
            printConfig(step, true, currentState, "", stk);
        }

        // Verifica os dois criterios de aceitacao
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

    // Empilha os caracteres de pushString em ordem reversa, de modo que
    // pushString[0] fique no topo ao final.
    // Ex.: "AZ" => empurra 'Z' primeiro, depois 'A' => 'A' fica no topo.
    static void applyPush(std::stack<char>& stk, const std::string& pushString) {
        for (int i = static_cast<int>(pushString.size()) - 1; i >= 0; --i)
            stk.push(pushString[static_cast<std::size_t>(i)]);
    }

    // Tenta disparar uma transicao epsilon a partir do estado e topo atuais.
    // Retorna true e atualiza estado/pilha se encontrar; false caso contrario.
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

    // Converte o conteudo da pilha em string para exibicao (topo primeiro).
    // Recebe a pilha por valor para nao modificar a original.
    std::string stackToString(std::stack<char> s) const {
        std::string result;
        while (!s.empty()) {
            result += s.top();
            s.pop();
        }
        return result.empty() ? "(vazia)" : result;
    }

    // Imprime a configuracao instantanea do automato no formato:
    // (estado, entrada_restante, pilha)
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
    // Cria o DPDA: simbolo de fundo 'Z', estado inicial 0 (q0), estado final 2 (q2)
    PDA pda('Z', 0, 2);
    pda.setStateName(0, "q0");
    pda.setStateName(1, "q1");
    pda.setStateName(2, "q2");

    // Fase de leitura dos 'a': empilha um 'A' para cada 'a' lido
    // δ(q0, a, Z) = (q0, AZ)  — primeiro 'a': empilha A sobre o fundo Z
    pda.addTransition(0, 'a', 'Z', 0, "AZ");
    // δ(q0, a, A) = (q0, AA)  — demais 'a': empilha mais um A
    pda.addTransition(0, 'a', 'A', 0, "AA");

    // Fase de leitura dos 'b': desempilha um 'A' para cada 'b' lido
    // δ(q0, b, A) = (q1, ε)   — primeiro 'b': muda de fase e desempilha A
    pda.addTransition(0, 'b', 'A', 1, "");
    // δ(q1, b, A) = (q1, ε)   — demais 'b': continua desempilhando A
    pda.addTransition(1, 'b', 'A', 1, "");

    // Aceitacao: quando todos os 'b' foram lidos e so resta o fundo Z na pilha,
    // a transicao epsilon desempilha Z e leva ao estado final q2 com pilha vazia.
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
