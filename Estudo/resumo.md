# Resumo de Estudo: Algoritmos Avançados (INF05010)

> Fonte da verdade: `Notas_de_aula.pdf` (Marcus Ritt) e `Algoritmos_quanticos.pdf`.
> Este resumo cobre os tópicos da prova: algoritmos randomizados, complexidades, classes
> (P, NP, PP, BPP, RP, etc.), a seção de algoritmos quânticos e os 4 algoritmos
> implementados no repositório com seus experimentos.

---

## Como usar este material

A prova tem cinco frentes. A tabela mapeia cada uma para onde estudar.

| Tópico da prova | Seção deste resumo | Fonte primária |
|---|---|---|
| Algoritmos randomizados | §1, §2 | Notas cap. 4 |
| Complexidade de algoritmos | §3 | Notas (vários) + Apêndice C |
| Classes P, NP, PP, BPP, ... | §1 | Notas §4.1 |
| Algoritmos quânticos | §6 | `Algoritmos_quanticos.pdf` |
| 4 algoritmos implementados + experimentos | §4, §5 | Relatórios + Notas §1.4, §1.6, §1.7, §4.2 |

> **Provas anteriores:** os PDFs em `Estudo/provas-antigas` (2017/1, 2021/1, 2022/2, 2025/1, com
> gabarito) foram incorporados. As questões resolvidas estão na **Parte J** de
> `perguntas_respostas.md`; os pontos que elas mais cobram já estão refletidos neste resumo
> (multiplicidade de cortes mínimos, potenciais de Johnson, pontos finos de Dijkstra, heaps).
> Padrão das provas: "**prova ou contra-exemplo**" e "**liste e justifique complexidades**".

---

# 1. Classes de complexidade e algoritmos randomizados

## 1.1 O que é um algoritmo randomizado

Um algoritmo randomizado usa **eventos aleatórios** durante a execução (modelo: máquina de
Turing probabilística, ou máquina RAM com comando `random(S)` que devolve elemento aleatório
de `S`). Vale a pena aceitar uma resposta errada com probabilidade baixa porque o algoritmo
randomizado costuma ser:

- mais simples;
- mais eficiente (às vezes é o algoritmo mais rápido conhecido);
- mais robusto (menos dependente da distribuição das entradas);
- a única alternativa conhecida para certos problemas.

**Dois tipos fundamentais:**

- **Monte Carlo:** responde corretamente apenas com certa probabilidade (pode errar). O tempo
  é fixo. Exemplo: teste de Miller-Rabin.
- **Las Vegas:** usa aleatoriedade só internamente, mas **sempre responde corretamente**; o que
  varia é o tempo de execução (esperado). Exemplo: Quickselect aleatorizado.

## 1.2 Definição base: a classe R(α, β)

Esta é a definição que organiza **todas** as classes randomizadas das notas. Não decore as
classes isoladamente; decore esta definição e derive as outras.

> **Definição (R(α, β)).** Seja `R(α, β)` a classe das linguagens `L` para as quais existe um
> algoritmo de decisão `A` em **tempo polinomial** tal que:
> - se `x ∈ L`, então `Pr(A(x) = "sim") ≥ α`;
> - se `x ∉ L`, então `Pr(A(x) = "não") ≥ β`.
>
> A probabilidade é sobre as sequências de bits aleatórios `r` (como `A` é polinomial em `|x|`,
> o número de bits aleatórios `|r|` também é).

A partir dela:

| Classe | Definição via R | Tipo de erro | Intuição |
|---|---|---|---|
| **RP** (randomized polynomial) | `R(1/2, 1)` | unilateral, no lado "sim" | se diz "sim", é sim; se diz "não", pode errar |
| **co-RP** | `R(1, 1/2)` | unilateral, no lado "não" | se diz "não", é não; se diz "sim", pode errar |
| **ZPP** (zero-error PP) | `RP ∩ co-RP` | nenhum (zero erro) | algoritmo Las Vegas; nunca erra |
| **BPP** (bounded-error PP) | `R(2/3, 2/3)` | bilateral **limitado** (erro 1/3) | erra nos dois lados, mas longe de 1/2 |
| **PP** (probabilistic polynomial) | `∪_{ε∈(0,1/2]} R(1/2+ε, 1/2+ε)` | bilateral **ilimitado** (erro 1/2+ε) | só "melhor que cara ou coroa" |

Pontos cruciais para a prova:

- **RP / co-RP têm erro unilateral**: uma das respostas é sempre confiável.
- **BPP tem erro bilateral mas limitado** longe de 1/2 (uma constante, ex. 1/3).
- **PP tem erro bilateral que pode estar arbitrariamente perto de 1/2** (o ε pode até diminuir
  com o tamanho da instância). Esta é a diferença essencial entre BPP e PP.

## 1.3 Amplificação de probabilidade

Repetir o algoritmo `k` vezes e combinar as respostas melhora a confiança.

- **Em RP, co-RP, BPP**: o erro diminui **exponencialmente** com o número de repetições `k`.
  - RP: repita `k` vezes, responda "sim" se alguma execução disse "sim". O erro cai como `(1/2)^k`.
  - BPP: repita `k` vezes e responda pela **maioria**; pela desigualdade de Chernoff o erro cai
    exponencialmente.
  - Teoremas formais: `R(α, 1) = R(β, 1)` para `0 < α, β < 1` (Teo. 4.1, base de RP), e
    `R(α, α) = R(β, β)` para `1/2 < α, β` (Teo. 4.2, base de BPP). Consequência:
    **RP = R(α, 1)** para qualquer `0 < α < 1` e **BPP = R(α, α)** para qualquer `1/2 < α`.
- **Em PP**: a amplificação **não** funciona da mesma forma, porque `ε` pode encolher com o
  tamanho da instância; não dá para garantir redução exponencial com um número constante de
  repetições. (Ponto clássico de prova: "por que não amplificamos PP?")

## 1.4 ZPP: três caracterizações equivalentes

`ZPP` (erro zero) admite três descrições equivalentes, todas nas notas:

1. **ZPP = RP ∩ co-RP** (definição).
2. **ZPP = classe das linguagens honestas.** Um algoritmo é *honesto* se responde "sim",
   "não" ou "não sei"; nunca erra quando responde; e `Pr(A(x) = "não sei") ≤ 1/2`.
3. **ZPP = classe das linguagens sem falha.** Um algoritmo é *sem falha* se sempre responde
   "sim" ou "não" corretamente, em **tempo polinomial esperado** (Las Vegas).

A ponte entre (2) e (3): repetir um algoritmo honesto até ele não responder "não sei" dá um
algoritmo sem falha cujo tempo esperado continua polinomial (`Σ_{k>0} k·2^{-k}·p(n) ≤ 2p(n)`).

## 1.5 Relações entre as classes (diagrama de Hasse)

As notas (Figura 4.1, p.153) dão o diagrama de inclusões. De baixo (mais fácil) para cima:

```
                    PP = co-PP
                   /    |     \
                 NP    BQP    co-NP        (relações com BQP/NP/co-NP em aberto: "?")
                   \    |     /
                  BPP = co-BPP
                   /          \
                 RP           co-RP
                   \          /
                     ZPP
                      |
                      P
```

Inclusões a memorizar (todas provadas nas notas):

- `P ⊆ ZPP ⊆ RP ⊆ BPP ⊆ PP` (e simetricamente com co-RP).
- **`RP ⊆ NP`** e **`co-RP ⊆ co-NP`** (Teo. 4.5). Intuição: um algoritmo RP que aceita com
  prob. ≥ 1/2 dá uma testemunha (a sequência aleatória `r` que faz aceitar); logo a verificação
  não-determinística de NP funciona.
- **`RP ⊆ BPP`** e **`co-RP ⊆ BPP`** (Teo. 4.6).
- **`BQP`** (bounded-error quantum polynomial, classe dos algoritmos quânticos) aparece entre
  BPP e PP, ao lado de NP e co-NP; sua relação com NP/co-NP é **aberta**. É o elo com a §6.

**Abundância de testemunhas (ligação com NP):** NP é a classe dos problemas cuja solução pode
ser *verificada* em tempo polinomial. Se, para uma instância, mais da metade das soluções
candidatas são testemunhas válidas, então "chutar" uma aleatoriamente e verificar dá um
algoritmo RP. Problemas assim têm *abundância de testemunhas*; é o que torna a randomização
poderosa (ex.: teste de identidade de polinômios). As notas tratam P e NP em nível de
pré-requisito (NP é caracterizada informalmente por verificação/testemunhas) e formalizam com
rigor apenas as classes randomizadas.

---

# 2. Os algoritmos randomizados do curso (cap. 4)

Quatro estudos de caso. Saiba a ideia, a complexidade e a probabilidade de cada um.

## 2.1 Teste de identidade de polinômios (exemplo de co-RP)

Dados `p(x)` e `q(x)` de grau ≤ `d`, decidir se `p ≡ q`. Comparar coeficientes na forma canônica
pode custar `Θ(d²)` se um deles está fatorado. Algoritmo randomizado:

```
identico(p, q):
  seleciona r aleatório em [1, 100d]
  se p(r) = q(r) retorne "sim"; senão retorne "não"
```

- Se `p ≡ q`, responde "sim" com certeza.
- Se `p ≢ q`, `p - q` é polinômio de grau `d` com no máximo `d` raízes, então
  `Pr(errar) = Pr(p(r)=q(r)) ≤ d/100d = 1/100`.
- Erro só no lado "sim" (responder "sim" quando são diferentes) → **co-RP**. Exemplo clássico de
  abundância de testemunhas.

## 2.2 Seleção aleatorizada (Quickselect) — também é o algoritmo implementado (ver §4.1)

Seleciona o `k`-ésimo menor elemento. Escolhe pivô `m` aleatório, particiona em `L` (< m) e
`R` (≥ m) e recorre só no lado certo. **Tempo esperado O(n)** (Las Vegas para o tempo).

Esboço da análise (recorrência): com probabilidade `1/n` o pivô deixa `|L| = i`, e o pior lado
tem `max{i, n-i}` elementos. Resolve-se
`T(n) ≤ cn + (1/n) Σ_i T(max{i, n-i})` e por indução `T(n) ≤ c'n`, logo **O(n) esperado**.

## 2.3 Corte mínimo aleatorizado (algoritmo de contração de Karger)

**Problema:** dado `G = (V, A)` não-direcionado com pesos, achar a partição `V = S ∪ S̄` que
minimiza o peso do corte. (Versão das notas: pesos unitários.)

**Determinístico:** `O(n)` aplicações de fluxo máximo (via árvore de Gomory-Hu ou cortes
`s`-`v`). Caro.

**Algoritmo de contração `cmr(G)`:**

```
enquanto G tem mais de 2 vértices:
  escolhe aresta {u,v} aleatoriamente
  contrai (identifica) u e v        (remove laços; mantém arestas múltiplas!)
retorna o corte definido pelos 2 vértices restantes
```

- **Lema-chave (4.5):** a probabilidade de um corte mínimo fixo (com `k` arestas) **sobreviver**
  a todas as `n-2` contrações é `Ω(1/n²)` (mais precisamente `≥ 2/(n(n-1))`).
  - Por que: como o corte mínimo é `k`, todo vértice tem grau `≥ k`, então o grafo tem `≥ kn/2`
    arestas; a chance de contrair uma aresta do corte numa iteração é pequena.
- **Teorema 4.7:** uma execução de `cmr` devolve um corte mínimo dado com prob. `Ω(n⁻²)`.
- **Amplificação:** repetindo `n² log n` vezes e ficando com o menor corte, acha-se o mínimo com
  prob. alta (`≥ 1 - n⁻²`). Custo total `O(n⁴ log n)`.
- **Karger-Stein (`cmr2`):** as contrações finais são as mais arriscadas. Faz duas recursões
  parando em `n' = ⌈1 + n/√2⌉` vértices. Acha o corte mínimo com prob. `Ω(1/log n)` e custa
  **`O(n² log n)`** (recorrência `T_n = 2T(f(n)) + O(n²)`, resolvida por Akra-Bazzi com `p = 2`,
  dando `Θ(n² log n)`).

## 2.4 Teste de primalidade (Monte Carlo, co-RP)

**Motivação:** criptografia (RSA) precisa de primos grandes. Pelo **teorema dos números primos**
(Hadamard / Vallée Poussin), `π(n) ~ n/ln n`, então a chance de um número aleatório em `[2,n]`
ser primo é `~ 1/ln n` (alta o suficiente). Falta testar primalidade rápido.

Evolução dos testes:

1. **Divisão por tentativa** (`Primo1`): testa divisores até `√n`. Custo `Ω(2^{t/2})` com
   `t = log n` bits → **exponencial** no tamanho da entrada. Ruim.
2. **Teste de Fermat** (`Primo2`): usa o **pequeno teorema de Fermat** (`a^{p-1} ≡ 1 mod p` para
   `p` primo, `(a,p)=1`). Sorteia `a`, testa `a^{n-1} ≡ 1 (mod n)`. Custo `O(log³ n)`.
   - **Problema: números de Carmichael** (ex. 561 = 3·11·17, 1105, 1729): compostos que passam no
     teste de Fermat para *toda* base coprima. Há infinitos deles (`C(n) > n^{2/7}`), então o
     teste de Fermat não tem garantia.
3. **Miller-Rabin** (`Primo3`): refina usando o **teorema da raiz modular** (para `p` primo,
   `x² ≡ 1 → x ≡ ±1 mod p`). Escreve `n-1 = 2^t·u` e checa o critério de *pseudo-primo forte*:
   `a^u ≡ 1` ou existe `i ∈ [0,t-1]` com `a^{2^i·u} ≡ -1`.
   - **Teorema (Monier/Rabin):** se `n` é composto ímpar, mais de `3/4` das bases `a` revelam que
     `n` é composto. Logo com `k` testes, `Pr(errar | n composto) ≤ (1/4)^k = 2^{-2k}`.
   - É **co-RP**: se diz "composto" (Não), está certo; só pode errar dizendo "primo".
   - Cota mais fina (Damgård et al.): para `n` com `k` bits, prob. de uma única falha `≤ k²·4^{2-√k}`.

---

# 3. Complexidade de algoritmos: ferramentas

## 3.1 Notação assintótica e contagem de operações

Os relatórios verificam complexidade **empiricamente** por dois caminhos:

- **Contagem de operações** (determinística): conta comparações, acessos, relaxações, etc.,
  e ajusta `operações ∝ n^k` em escala log-log; o expoente `k` empírico deve bater com a teoria.
- **Tempo de parede**: ajusta modelos candidatos (`an+b`, `an ln n + b`, `an²+b`) por mínimos
  quadrados e escolhe o de maior `R²`. As **constantes** decidem o vencedor prático dentro de
  uma mesma classe assintótica.

## 3.2 Análise de recorrências (Apêndice C)

O curso usa **dois teoremas** (não o "teorema mestre" clássico):

- **Akra-Bazzi-Leighton (Teo. C.1):** para
  `T(x) = Σ_{i} a_i·T(b_i·x + h_i(x)) + g(x)` (com `a_i > 0`, `0 < b_i < 1`, perturbações `h_i`
  pequenas), tem-se
  `T(x) = Θ( x^p · (1 + ∫₁^x g(u)/u^{p+1} du) )`,
  onde `p` resolve a **equação característica** `Σ_i a_i·b_i^p = 1`.
  - Aplicação no curso: Karger-Stein, `2·(1/√2)^p = 1 → p = 2 → Θ(n² log n)`.
- **Graham et al. (Teo. C.2):** para recorrências do tipo subtrativo
  `T(n) = Σ_i α_i·T(n - d_i)`, seja `α` a raiz de maior módulo (multiplicidade `l`) do polinômio
  característico `z^d - α₁z^{d-d₁} - ... - α_k z^{d-d_k}`. Então `T(n) = Θ(n^l·α^n)`.

## 3.3 Padrões de complexidade que reaparecem

- Recursão que **contrai** (soma das frações dos subproblemas < 1) → linear. Se a soma = 1 →
  sem garantia (caso `g=3` da mediana das medianas, §4.1).
- Fila de prioridade com heap binário: `insert/decrease-key` `O(log n)`, `extract-min`
  `O(log n)`; heap `k`-ário: `sift_up` `O(log_k n)`, `sift_down` `O(k·log_k n)` (§4.2).
- Cota inferior `Ω(log n)` para `insert`+`extract-min` (senão ordenaríamos abaixo de `n log n`).

## 3.4 Filas de prioridade (heaps): tipos e complexidades

Heaps aparecem como ferramenta (Dijkstra, Prim) e como tema próprio em prova. Operações:
`insert`, `deletemin` (extract-min), `decreasekey` (update).

| Heap | insert | deletemin | decreasekey |
|---|---|---|---|
| Binário | `O(log n)` | `O(log n)` | `O(log n)` |
| `k`-ário | `O(log_k n)` | `O(k·log_k n)` | `O(log_k n)` |
| Binomial | `O(log n)` (amort. `O(1)`) | `O(log n)` | `O(log n)` |
| Fibonacci | `O(1)` | `O(log n)` amort. | `O(1)` amort. |
| Oco (hollow) | `O(1)` | `O(log n)` amort. | `O(1)` amort. |

- **Por que heaps com `decreasekey` `O(1)` amortizado importam:** Dijkstra/Prim fazem muitos
  `update`; Fibonacci e oco dão `O(m + n log n)`.
- **Cuidado com "ideias de melhoria" (armadilha de prova):** alterações que parecem inocentes
  podem **quebrar a forma** do heap ou suas garantias. Exemplos (min heap):
  - `deletemin` trocando a raiz por `∞`, descendo e removendo "uma folha": **não funciona**, pois
    a folha removida pode não ser a **última**, desbalanceando a árvore.
  - **Trocar irmãos** (com suas subárvores) **preserva** a propriedade de heap (só reordena filhos
    do mesmo pai).
  - **Permutar todas as folhas** **não** preserva (folhas podem violar a propriedade vs. os pais).

---

# 4. Os 4 algoritmos implementados + experimentos

Esta é a parte mais cobrada sobre o repositório. Para cada um: o problema, os algoritmos
comparados, a teoria e **o que os experimentos de fato mostraram** (inclusive surpresas).

## 4.1 Seleção do k-ésimo elemento (`Selecao/`)

**Problema:** achar o `k`-ésimo menor elemento de um vetor não ordenado.

**Algoritmos comparados:**

| Algoritmo | Complexidade (comparações) | Observação |
|---|---|---|
| Seleção Ingênua (ordena e indexa) | `Θ(n log n)` | independente de `k` |
| Quickselect aleatorizado | `O(n)` esperado / `Θ(n²)` pior | pivô aleatório; Lomuto |
| Mediana das Medianas (`g ≥ 5`) | `Θ(n)` pior caso* | *exige tratar duplicatas |
| Mediana das Medianas (`g = 3`) | sem garantia linear | `1/g + (3g-1)/4g = 1` |
| `std::nth_element` (introselect) | `O(n)` pior caso | Quickselect + recuo p/ heapselect |

**Teoria da mediana das medianas:** divide em grupos de `g`, acha a mediana de cada grupo,
recorre na mediana das medianas. Garante `≈ n(g+1)/(4g)` elementos de cada lado; recorrência
`T(n) ≤ T(n/g) + T((3g-1)/4g · n) + O(n)`, que **contrai sse `1/g + (3g-1)/(4g) < 1`, ou seja,
`g ≥ 5`**. Para `g = 3` a soma é exatamente 1 (sem garantia).

**Resultados experimentais (E1 a E9):**

- **E1/E2 (contagens e tempo):** expoentes empíricos confirmam as classes: Ingênua `1,12`
  (≈ `n log n`), Quickselect `0,99`, Mediana das Medianas `1,04` (lineares). Mas as **constantes**
  diferem: Quickselect `9,7 ns/elem`, `nth_element` `8,7 ns/elem`, Mediana das Medianas
  `25,7 ns/elem` (≈ 2,7× mais lenta apesar de linear, por causa de ordenar grupos + recursão
  aninhada).
- **E3 (distribuição do Quickselect):** média e desvio crescem ambos linearmente em `n`, então
  **variância ∝ n²** e o **coeficiente de variação CV ≈ 0,28 é constante** (variabilidade
  relativa não cresce com a escala). Cauda direita suave (assimetria 0,6 a 0,8): sequências
  ocasionais de pivôs ruins.
- **E4 (dependência de `k`):** Ingênua é plana (ordena tudo); Mediana das Medianas quase plana
  (divisão garantida ≈30/70); Quickselect tem dependência moderada, mais barato nos extremos.
- **E5 (tamanho de grupo `g`):** curva de custo em **forma de U**. Ótimo empírico: comparações
  em `g=7`, tempo em `g=9`. `g=5` é competitivo mas não ótimo nesta máquina.
- **E6 (anomalia `g=3`):** apesar de a recorrência não contrair, **`g=3` permanece linear em
  entradas benignas até `n=10⁶`**. A degradação de `g=3` é fenômeno **estritamente de pior caso**.
- **E7 (robustez / duplicatas) — ACHADO PRINCIPAL:** sob `alleq` (todos iguais) e baixa
  cardinalidade, a **partição de Lomuto (duas vias) manda tudo para um lado**:
  - Quickselect degrada para **`Θ(n²)`** (expoente 2,00).
  - **Mediana das Medianas degrada para expoente 3,85 (pior que cúbico)**, por causa da recursão
    aninhada de `get_pivot`. A garantia linear da literatura pressupõe partição de **três vias**
    que isole as chaves iguais ao pivô; a implementação de duas vias **não preserva** a garantia.
    (Decisão consciente: reportar como achado, não mascarar.)
  - `std::nth_element` resiste (expoente 0,99).
- **E8/E9 (vencedor prático e `nth_element`):** para entrada aleatória **não há vencedor absoluto
  entre Quickselect e `nth_element`** (empatados, lineares). O `std::nth_element` (introselect =
  Quickselect com mediana-de-3 + recuo para heapselect quando a recursão fica funda) é o único
  que fica **linear em todas as distribuições** (≈ 2n a 3n comparações até em `alleq`),
  justificando por que as bibliotecas reais não usam nenhum dos três algoritmos puros.

**Ordenamento prático:** `nth_element ≈ Quickselect < Mediana das Medianas < Ingênua`.

## 4.2 Dijkstra com heap k-ário (`Dijkstra/`)

**Problema:** caminhos mais curtos de uma origem `s` para todos os vértices, pesos `≥ 0`.

**Estrutura central:** heap `k`-ário com array `pos_` que dá a posição de cada vértice em `O(1)`
(essencial para `decrease_key`).

**Complexidades das operações:**

| Operação | Custo | Por quê |
|---|---|---|
| Busca de vértice (via `pos_`) | `O(1)` | mapeamento direto |
| `sift_up` (Insert, Decrease-key) | `O(log_k n)` | sobe comparando com o pai |
| `sift_down` (Extract-min) | `O(k·log_k n)` | em cada nível compara `k` filhos |

**Complexidade total de Dijkstra:** `O(n) + n·deletemin + n·insert + m·update`, que com o heap
`k`-ário dá **`O(n·k·log_k n + m·log_k n)`**.

**Caso médio (Noshita, 1985):** com pesos aleatórios, o número de operações `update` é em média
**`n·ln(m/n)`** (a chance de uma aresta causar relaxação é a de ela ser um mínimo local numa
permutação aleatória; o número esperado de mínimos locais é `H_k - 1 ≤ ln k`).

**Pontos finos de Dijkstra (recorrentes em prova):**

- **Critérios de parada para caminho `s`-`t`:** dá para parar quando `t` sai da fila. Mais
  agressivo: parar ao processar **qualquer** vértice `v` com `d_v = d_t` é **correto** (Dijkstra
  processa em ordem crescente, então `d_t` já é mínima). Mas parar quando `t` é **atualizado**
  (decreasekey em `t`) é **errado**: a distância de `t` ainda pode diminuir depois (contra-exemplo
  triângulo `sv=1, vt=1, st=3`).
- **Mudança de peso de uma aresta:** rodar Dijkstra com as distâncias antigas corretas dá as
  novas corretas **se o peso diminui** (na versão que insere todos os vértices), mas **não
  necessariamente se o peso aumenta** (as distâncias antigas ficam subestimadas).
- **Barreira de ordenação (`Ω(n log n)`):** como Dijkstra entrega os vértices em **ordem de
  distância** a custo `O(1)` extra por vértice, ele ordena; logo nenhum algoritmo baseado em
  comparações que também ordene pode ser mais rápido que `Ω(n log n)`. (Há algoritmos mais
  rápidos que produzem **só** as distâncias, sem ordenar; Duan et al. 2025.)
- **Distâncias negativas:** Dijkstra **não** funciona com pesos negativos; usa-se Bellman-Ford
  ou Johnson (ver §4.4), corretos se não há ciclos de custo negativo.

**Resultados experimentais:**

- **Grau ótimo do heap:** trade-off entre profundidade (favorece `k` pequeno, `sift_up` barato)
  e largura (favorece `k` grande, mas `sift_down` mais caro). Grafos **densos** (mais `update`)
  se beneficiam de `k` maior. **Ótimo experimental: `k = 8`**, especialmente em grafos densos;
  acima de `k=16` o custo de `sift_down`/Insert supera a economia em `update`.
- **Validação da complexidade do heap:** a fração de sifts realizada nunca ultrapassa o limite
  teórico (`log_k n`).
- **Caso médio:** a curva empírica de `update` acompanha o valor esperado de Noshita
  `n·ln(m/n)`; `update` cresce com `k` (árvores mais largas).
- **Escalonamento assintótico:** ao normalizar `T / ((n+m)·log n)`, as curvas ficam
  praticamente **horizontais** mesmo com o grafo crescendo → a implementação escala conforme a
  teoria.
- **Mundo real (DIMACS, grafo `USA.gr`, ~24 milhões de vértices):** o algoritmo escalou
  corretamente; o **gargalo é a E/S de disco** (leitura do arquivo), não o cômputo. RAM estável.

## 4.3 Fluxo máximo s-t (`Fluxos/`)

**Problema:** fluxo `s`-`t` máximo num grafo direcionado capacitado. Base teórica:

- **Grafo residual `G_f`:** arcos "forward" com capacidade `c_a - f_a` e arcos "backward" com
  capacidade `f_a`. Aumentar fluxo = achar caminho `s`-`t` em `G_f` e empurrar o gargalo.
- **Teorema fluxo máximo = corte mínimo (Teo. 1.10):** o valor do fluxo máximo `s`-`t` é igual
  ao valor do corte `s`-`t` mínimo. Todo corte é um limite superior do fluxo; quando
  Ford-Fulkerson para, o conjunto alcançável em `G_f` define um corte saturado = fluxo máximo.

**Multiplicidade de fluxos e cortes (MUITO cobrado em prova):**

- Pode haver **vários fluxos máximos e um único corte mínimo** (ex.: diamante
  `s → {a,b} → c → t` com capacidades 1).
- Pode haver **um único fluxo máximo e vários cortes mínimos** (ex.: caminho `s → · → t` com
  capacidades 1: cada aresta é um corte mínimo).
- **Qual corte mínimo os algoritmos do tipo Ford-Fulkerson devolvem?** O **menor** corte mínimo
  `C_min`, definido por `X` = vértices **alcançáveis a partir de `s` no grafo residual** quando o
  algoritmo para. Como a busca parte de `s`, não há arco saindo de `X`, e `C_min ⊆ C` para todo
  corte mínimo `C` (ou seja, `C_min = ∩` de todos os cortes mínimos).
- **Sobre um arco `uv`:** o corte mínimo que **separa** `u` de `v` satisfaz `C ≥ c_uv` (o arco
  `uv` obrigatoriamente faz parte do corte).
- **Árvore de Gomory-Hu:** árvore que codifica **todos** os cortes mínimos (o corte mínimo `uv` é
  a aresta de menor peso no caminho `u`-`v` na árvore). Construída com `n-1` cálculos de corte
  mínimo, logo polinomial.

**Seis algoritmos da família Ford-Fulkerson:**

| Sigla | Algoritmo | Complexidade | Busca |
|---|---|---|---|
| FF | Ford-Fulkerson DFS | `O(mC)` | DFS (pode aumentar 1 por fase) |
| RDFS | Ford-Fulkerson randomizado | `O(mC)` | DFS com embaralhamento |
| EK | Edmonds-Karp | `O(nm²)` | BFS (caminho mais curto) |
| Di | Dinitz | `O(n²m)` | grafo de níveis + fluxo bloqueante |
| EC | Escalonamento de capacidade | `O(m² log C)` | BFS restrita a arcos `≥ Δ` |
| FP | Caminho mais gordo (fattest) | `O(m² log C)` | Dijkstra modificado (maximiza gargalo) |

Pontos teóricos:

- **FF é apenas pseudo-polinomial** (`C` pode ser enorme) e pode nem terminar com capacidades
  irracionais. **EK** conserta usando BFS: `O(nm)` iterações (a distância `s`-`v` em `G_f`
  cresce monotonicamente, lema 1.20), logo `O(nm²)`.
- **Fattest path:** cada aumento melhora o fluxo em `≥ OPT/m`, logo `O(m log(mU))` fases.

**Foco do relatório: métricas de "defeito"** (quão frouxos são os limites teóricos):

- `r = F / F̄`: razão entre fases reais `F` e o limite teórico `F̄`.
- `s̄`: fração média de vértices tocados por fase.
- `t̄`: fração média de arcos avaliados por fase.

**Resultados experimentais (famílias `BasicLine` e `DoubleExpLine`, geradas pelo washington):**

- **Os limites teóricos são MUITO frouxos:** `r` fica entre `10⁻⁵` e `10⁻²` em tudo. Ex.:
  EK com `F̄ = nm/2 ≈ 4×10⁸` executa só `~4700` fases (5 ordens de magnitude de folga).
- **Dinitz é o melhor na prática:** menos fases (na `BasicLine` satura a rede em **1 única
  fase**!), curva de tempo mais baixa e estável. Na `DoubleExpLine`, `F` cresce só
  **logaritmicamente** com `n` (de 8 a 18 enquanto `n` vai de 1002 a 20002), contra `F̄ = n`.
- **`t̄` revela o mecanismo:** EK (BFS) toca quase todos os arcos por fase (`t̄ ≈ 0,97`); FF
  (DFS) ~metade (`0,48`); **RDFS toca pouquíssimos** (`0,016`, caminhos curtos por
  embaralhamento); **Dinitz é o único com `t̄ > 1`** (2,8 a 4,1), pois cada fase faz **várias**
  travessias (fluxo bloqueante).
- **Tensão fundamental:** poucas fases ↔ muitos arcos por fase (EK, Dinitz); muitas fases ↔
  poucos arcos por fase (RDFS). Dinitz vence combinando pouquíssimas fases com custo por fase
  controlado.
- **Fattest Path decepciona:** mesmo com `F̄` mais preciso, paga caro pela manutenção do
  **Max-Heap** (sobrecarga de estrutura de dados), ficando mais lento que EC apesar do mesmo
  número de fases.

## 4.4 Emparelhamento em grafos bipartidos (`Emparelhamentos/`)

**Problema:** emparelhamento máximo (não ponderado) e de **peso máximo** (ponderado) em grafo
bipartido com `2n` vértices e `m = n^α` arestas (`1 ≤ α ≤ 2`; `α` é o expoente de densidade).

Base teórica:

- **Caminho M-aumentante:** caminho alternante entre pontas livres. **Teorema de Berge:** um
  emparelhamento é máximo sse não há caminho M-aumentante.
- **Hopcroft-Karp (Teo. 1.22):** agrupa aumentos em **fases**; em cada fase, uma BFS constrói uma
  rede em camadas e uma DFS acha um conjunto **máximo de caminhos aumentantes mínimos
  vertex-disjuntos**. Bastam **`O(√n)` fases** (lemas 1.29: o comprimento mínimo cresce ≥ 2 por
  fase; 1.30: no máximo `√n` fases). Total `O(m√n)`.
- **Teorema de König (Teo. 1.16):** em grafo bipartido, tamanho do emparelhamento máximo =
  tamanho da cobertura de vértices mínima. (Dual: independente máximo = complemento da cobertura.)
- **Ponderado:** caminho aumentante de menor custo (custo negado) na rede residual. Para suportar
  pesos negativos: **Bellman-Ford** direto, ou **Dijkstra + potenciais de Johnson** (mantêm
  pesos transformados `≥ 0`).
- **Condição de corretude (BF e Johnson):** ambos dão resultado correto **se não há ciclos de
  custo negativo**.
- **Potencial de Johnson (definição exata):** uma função `p: V → ℝ` tal que `d_uv ≥ p_v - p_u`
  para todo arco. Então o custo transformado `d'_uv = d_uv - (p_v - p_u) ≥ 0` é não-negativo.
  Propriedade-chave: um caminho `s`-`v` mais curto em `d` continua mais curto em `d'`, então de
  posse de um potencial dá para usar **Dijkstra** (mais eficiente que Bellman-Ford). No matcher,
  os potenciais são atualizados com as distâncias após cada aumento para manter a invariância.

**Quatro algoritmos comparados:**

| Algoritmo | Complexidade | Expoente em `n` |
|---|---|---|
| Simple (caminhos aumentantes / DFS) | `O(n·m) = O(n^{1+α})` | `1+α` |
| Hopcroft-Karp | `O(√n·m) = O(n^{0,5+α})` | `0,5+α` |
| Húngaro + Bellman-Ford (BF) | `O(n²·m) = O(n^{2+α})` | `2+α` |
| Húngaro + Johnson/Dijkstra (JD) | `O(n·m·log n) = O(n^{1+α} log n)` | `1+α (+log)` |

**Resultados experimentais (não ponderados, U1 a U4, C1):**

- **U1 — surpresa:** o número de **fases do HK é praticamente constante** (1 a 2) para
  `α ≥ 1,5`, **não `O(√n)`**. Nos grafos densos gerados, uma fase BFS+DFS já acha o
  emparelhamento perfeito. Para `α=1,0` cresce devagar (expoente 0,16).
- **U3/C1 — cruzamento:** para `α = 1,0` (esparso) o **Simple é ~5× mais rápido** que HK (o
  overhead de montar a rede em camadas não compensa quando os caminhos são curtos). Para
  `α ≥ 1,5` o HK domina, com vantagem que cresce com `n` (de ~54× a ~1084× para `α=2,0` entre
  `n=500` e `n=10000`). Como HK roda `O(1)` fases em grafos densos, sua complexidade efetiva é
  `O(m) = O(n²)`, não `O(n^{2,5})`.

**Resultados experimentais (ponderados, W1 a W4, C2):**

- **W1/W4:** BF e JD produzem **exatamente** as mesmas aumentações e os mesmos valores (até em
  regime totalmente negativo), evidência indireta de que os potenciais de Johnson preservaram a
  invariância `d' ≥ 0`.
- **W3/C2 — achado mais contraintuitivo:** para `α = 2,0` (denso) **BF é ~10× mais rápido que
  JD** (e a vantagem cresce com `n`), apesar de JD ter melhor complexidade assintótica. Dois
  motivos: (1) **terminação antecipada** do BF (flag `relaxed`) derruba o expoente empírico de
  4,0 para ~3,0, e o acesso sequencial à matriz tem ótima localidade de cache; (2) o **heap do
  JD** com `O(n²)` entradas tem alto custo constante e péssima localidade. A vantagem de JD é
  máxima em `α ≈ 1,25` e o cruzamento ocorre por volta de `α ≈ 1,75`.

**Implicações práticas:** não ponderado → HK para `α ≥ 1,5`, Simple para `α=1,0`. Ponderado →
JD para grafos esparsos/médios (`α ≤ 1,5`), **BF para grafos densos** (`α ≈ 2,0`).

---

# 5. Lições transversais dos experimentos

Temas que aparecem nos quatro relatórios e são ótimos para questões dissertativas:

1. **Limites de pior caso são frequentemente frouxos.** Fluxos: `r` até 10⁻⁵. Matchings: HK
   roda `O(1)` fases em vez de `O(√n)`. A teoria dá garantias, não previsões de caso médio.
2. **A mesma classe assintótica, constantes diferentes.** Quickselect vs Mediana das Medianas
   (ambos `O(n)`, mas 2,7× de diferença). A constante decide o vencedor prático.
3. **Detalhes de implementação revertem previsões assintóticas.** BF com terminação antecipada
   bate JD em grafos densos; o overhead de heap (Fattest Path, JD denso) custa caro;
   localidade de cache importa.
4. **A garantia teórica pode depender de uma hipótese silenciosa.** Mediana das Medianas só é
   linear no pior caso se a partição tratar duplicatas (três vias); com Lomuto (duas vias) ela
   degrada para expoente ~3,85. Leia as letras miúdas das provas.
5. **Aleatoriedade dá robustez.** RDFS (fluxos) imuniza contra ordens adversárias; pivô
   aleatório do Quickselect é imune a arranjos fixos (só duplicatas o derrubam).
6. **Caso médio formalizado.** Noshita (`n ln(m/n)` updates no Dijkstra) e a análise do
   Quickselect (`CV` constante) mostram como modelar o caso médio, não só o pior caso.

---

# 6. Algoritmos quânticos (`Algoritmos_quanticos.pdf`)

Escopo da prova = exatamente o que está no PDF: qubit, postulados, esfera de Bloch, portas,
múltiplos qubits e o algoritmo de Deutsch. (Não há Grover/Shor no material.)

## 6.1 Os quatro postulados

1. **Espaço de estados:** um sistema quântico vive num espaço de Hilbert `H` (espaço vetorial
   complexo com produto interno); estados são **vetores unitários** de `H`. Notação de Dirac:
   ket `|ψ⟩` (vetor coluna), bra `⟨ψ|` (conjugado-transposto, vetor linha).
2. **Evolução:** um estado evolui por um **operador unitário** `U` (solução da equação de
   Schrödinger `iℏ d/dt |ψ⟩ = H|ψ⟩`, `H` hermitiano).
3. **Medição (regra de Born):** descrita por operadores `{M_m}` com `Σ_m M_m†M_m = I`. A
   probabilidade de medir `m` é `P(m) = ⟨ψ|M_m†M_m|ψ⟩`; depois da medição o estado colapsa para
   `M_m|ψ⟩ / √P(m)`.
4. **Sistemas compostos:** o espaço de `n` sistemas é o **produto tensorial**
   `H₁ ⊗ ... ⊗ H_n`, e o estado é `|ψ₁⟩ ⊗ ... ⊗ |ψ_n⟩`.

## 6.2 O qubit

Um qubit é um sistema de dimensão `N = 2`, com base `|0⟩` e `|1⟩`:

`|ψ⟩ = α₀|0⟩ + α₁|1⟩`, com `α_i ∈ ℂ` e **`|α₀|² + |α₁|² = 1`** (normalização, postulado 1).

- **Medição (postulado 3):** observa-se `|i⟩` com probabilidade `|α_i|²`.
- **Superposição:** ex. `|+⟩ = (|0⟩ + |1⟩)/√2`. Convenção das notas: **omitir a normalização**
  para simplificar (lembrar de renormalizar antes de medir).
- **Esfera de Bloch:** a fase global é fisicamente irrelevante (`|zα|² = |α|²` se `|z|=1`), então
  sobram **2 parâmetros reais**: `|ψ⟩ = cos(θ/2)|0⟩ + sin(θ/2)e^{iφ}|1⟩`. Um qubit é um ponto na
  esfera: `θ ∈ [0,π]` (polar a partir de `|0⟩`), `φ ∈ [0,2π]` (azimutal). (`n` qubits têm
  `2·2ⁿ - 2` graus de liberdade reais.)

## 6.3 Portas (operadores) de um qubit

Operadores devem ser unitários. Os principais:

- **Identidade e matrizes de Pauli:**
  `I = [[1,0],[0,1]]`, `X = [[0,1],[1,0]]` (NOT: `X|i⟩ = |1-i⟩`),
  `Y = [[0,-i],[i,0]]`, `Z = [[1,0],[0,-1]]`.
- **Hadamard:** `H = (1/√2)[[1,1],[1,-1]]`. Cria superposição:
  `H|0⟩ = |+⟩`, `H|1⟩ = |-⟩`, e `H² = I`.
- Relações úteis (exercício): `H = (X+Z)/√2`, `XY = iZ`, e `σ_i² = I`.

## 6.4 Múltiplos qubits, produto tensorial e portas controladas

- `n=2`: base `|00⟩, |01⟩, |10⟩, |11⟩`, dimensão `N = 2ⁿ = 4`. Operadores são matrizes `N×N`.
- **Strings de Pauli:** ex. `X ⊗ Z`. Aplicando a `|01⟩`: `(X⊗Z)|01⟩ = X|0⟩ ⊗ Z|1⟩ = -|11⟩`.
- **Portas controladas:** um qubit de **controle** decide se a porta age no **alvo**. O CNOT
  (NOT controlado) faz `|a⟩|b⟩ → |a⟩|a⊕b⟩` (notação: ponto preto no controle, ⊕ no alvo).

## 6.5 O algoritmo de Deutsch

**Problema:** dada `f: {0,1} → {0,1}` por um oráculo, descobrir `f(0) ⊕ f(1)` (isto é, se `f` é
constante ou balanceada). **Classicamente** são precisas **2 consultas**; **quanticamente, 1**.

- **Oráculo (unitário):** `U_f|x⟩|y⟩ = |x⟩|y ⊕ f(x)⟩`.
- **Circuito:** prepara `|0⟩|1⟩`, aplica Hadamard nos dois, depois `U_f`, depois Hadamard no
  primeiro qubit e mede.
- **Resultado:** medindo o primeiro qubit obtém-se **0 se `f(0) = f(1)`** (constante) e **1 se
  `f(0) ≠ f(1)`** (balanceada). O truque é o *phase kickback*: `|v⟩ - |1⊕v⟩ = (-1)^v|-⟩`, que
  transforma a fase `(-1)^{f(0)} ± (-1)^{f(1)}` num resultado mensurável.
- **Significado:** primeiro exemplo de **vantagem quântica** (uma consulta resolve o que
  classicamente exige duas), graças a superposição + interferência. Liga-se à classe **BQP** do
  diagrama da §1.5.

---

# 7. Checklist final de revisão

- [ ] Sei derivar RP, co-RP, ZPP, BPP, PP a partir de `R(α,β)` e dizer o tipo de erro de cada.
- [ ] Sei por que amplifica RP/BPP mas não PP.
- [ ] Sei as 3 caracterizações de ZPP e o diagrama de Hasse (com BQP).
- [ ] Distingo Monte Carlo de Las Vegas e dou exemplos (Miller-Rabin vs Quickselect).
- [ ] Sei a ideia, complexidade e probabilidade de: identidade de polinômios, Quickselect,
      corte mínimo de Karger (+ Karger-Stein), Miller-Rabin (+ Carmichael).
- [ ] Sei resolver recorrências com Akra-Bazzi-Leighton (eq. característica `Σ a_i b_i^p = 1`).
- [ ] Sei, para cada um dos 4 algoritmos implementados: o problema, os métodos comparados, as
      complexidades e **os resultados/surpresas dos experimentos**.
- [ ] Sei os 4 postulados, o qubit, a esfera de Bloch, Pauli/Hadamard e o algoritmo de Deutsch.
