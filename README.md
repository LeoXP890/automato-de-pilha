# Autômato de Pilha (DPDA) — C++

Implementação de um Autômato de Pilha Determinístico (DPDA) que reconhece a linguagem:

```
L = { aⁿbⁿ | n ≥ 1 }
```

Ou seja, cadeias com **n** letras `a` seguidas de **n** letras `b` (pelo menos uma de cada).

---

## Definição Formal

| Componente | Valor |
|---|---|
| Estados | Q = {q0, q1, q2} |
| Alfabeto de entrada | Σ = {a, b} |
| Alfabeto da pilha | Γ = {Z, A} |
| Estado inicial | q0 |
| Símbolo inicial da pilha | Z |
| Estados finais | {q2} |

### Tabela de Transições

| δ | Estado | Entrada | Topo da pilha | Próx. estado | Empilha |
|---|---|---|---|---|---|
| 1 | q0 | a | Z | q0 | AZ |
| 2 | q0 | a | A | q0 | AA |
| 3 | q0 | b | A | q1 | ε |
| 4 | q1 | b | A | q1 | ε |
| 5 | q1 | ε | Z | q2 | ε |

A cadeia é **aceita** quando o autômato atinge o estado final **q2** com a pilha vazia.

---

## Como Compilar e Executar

### Pré-requisito

- Compilador **g++** com suporte a C++17 (GCC/MinGW)

### Compilar

```bash
g++ -std=c++17 -o automato atividade.c++
```

### Executar

```bash
./automato
```

No Windows:

```cmd
automato.exe
```

---

## Exemplo de Uso

```
=== Simulador de Automato de Pilha ===
Linguagem reconhecida: { a^n b^n | n >= 1 }

Digite uma cadeia (ou 'sair' para encerrar): aabb

Config. inicial: (q0, aabb, Z)
Passo 1: (q0, abb, AZ)
Passo 2: (q0, bb, AAZ)
Passo 3: (q1, b, AZ)
Passo 4: (q1, , Z)
Passo 5 [epsilon]: (q2, , (vazia))
Cadeia ACEITA por estado final e pilha vazia.
```

### Casos de teste

| Cadeia | Resultado |
|---|---|
| `ab` | ACEITA |
| `aabb` | ACEITA |
| `aaabbb` | ACEITA |
| `aab` | REJEITA |
| `abb` | REJEITA |
| `ba` | REJEITA |
| `a` | REJEITA |
| (vazia) | REJEITA |

---

## Estrutura do Código

- **`TransitionKey`** — chave da tabela de transições (estado, símbolo de entrada, topo da pilha)
- **`TransitionValue`** — valor da transição (próximo estado, string a empilhar)
- **`class PDA`** — encapsula o autômato: tabela de transições, simulação passo a passo e verificação de aceitação
- **`main()`** — define o DPDA e executa o loop de leitura de cadeias
