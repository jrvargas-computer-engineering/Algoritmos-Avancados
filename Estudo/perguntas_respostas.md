# Perguntas e Respostas: Algoritmos Avançados (INF05010)

> Banco de questões para a prova, baseado em `Notas_de_aula.pdf` e `Algoritmos_quanticos.pdf`.
> Mistura perguntas conceituais (definições), de cálculo de complexidade e dissertativas sobre
> os experimentos dos 4 algoritmos implementados.
>
> **Como estudar:** tente responder antes de ler a resposta. As respostas marcadas com
> **(experimento)** cobram dados dos relatórios do repositório. A **Parte J** traz as
> **questões reais das provas anteriores** (pasta `Estudo/provas-antigas`) com o gabarito
> oficial: é o material mais próximo do que costuma cair, comece por ela na reta final.

Índice:
- [Parte A: Algoritmos randomizados](#parte-a)
- [Parte B: Classes de complexidade](#parte-b)
- [Parte C: Complexidade e recorrências](#parte-c)
- [Parte D: Seleção do k-ésimo elemento](#parte-d)
- [Parte E: Dijkstra com heap k-ário](#parte-e)
- [Parte F: Fluxo máximo](#parte-f)
- [Parte G: Emparelhamentos bipartidos](#parte-g)
- [Parte H: Algoritmos quânticos](#parte-h)
- [Parte I: Questões integradoras](#parte-i)
- [Parte J: Questões de provas anteriores (com gabarito)](#parte-j)

---

<a name="parte-a"></a>
# Parte A: Algoritmos randomizados

### A1. O que é um algoritmo randomizado e quais suas vantagens?
É um algoritmo que usa **eventos aleatórios** na execução (modelo: máquina de Turing
probabilística ou RAM com `random(S)`). Vantagens: costuma ser mais **simples**, mais
**eficiente** (às vezes o mais rápido conhecido), mais **robusto** (menos dependente da
distribuição da entrada) e às vezes é a **única** alternativa conhecida. Em troca, pode dar
resposta errada com probabilidade baixa, o que é aceitável.

### A2. Diferencie algoritmos Monte Carlo e Las Vegas.
- **Monte Carlo:** pode **errar** (responde correto só com certa probabilidade), mas tem tempo
  fixo. Ex.: Miller-Rabin, teste de identidade de polinômios.
- **Las Vegas:** usa aleatoriedade só internamente e **sempre acerta**; o que varia é o **tempo**
  (analisamos o tempo esperado). Ex.: Quickselect aleatorizado.

### A3. Descreva o teste randomizado de identidade de polinômios e classifique seu erro.
Para `p(x) ≡ q(x)?` de grau ≤ `d`: sorteia `r` em `[1,100d]`, responde "sim" se `p(r)=q(r)`,
senão "não". Se `p ≡ q`, acerta sempre. Se `p ≢ q`, `p-q` tem ≤ `d` raízes, então
`Pr(p(r)=q(r)) ≤ d/100d = 1/100`. O erro só acontece no lado "sim" (dizer iguais sendo
diferentes) → pertence a **co-RP**. É o exemplo clássico de **abundância de testemunhas**.

### A4. Explique o algoritmo de contração de Karger para corte mínimo.
Repete: escolhe uma aresta `{u,v}` aleatória, **contrai** (identifica `u` e `v`, removendo laços
mas **mantendo arestas múltiplas**), até sobrarem 2 vértices; o corte são esses 2 super-vértices.
Ideia: uma aresta fora do corte mínimo pode ser contraída sem perder o corte; a chance de
contrair por acaso uma aresta do corte é baixa.

### A5. Qual a probabilidade de o algoritmo de Karger achar um corte mínimo fixo, e como amplificar?
A probabilidade de um corte mínimo sobreviver às `n-2` contrações é **`Ω(1/n²)`** (`≥ 2/(n(n-1))`)
porque, como o grau mínimo é `≥ k` (= tamanho do corte), há muitas arestas e a chance de
contrair uma do corte é pequena (Lema 4.5). **Amplificação:** repete `n² log n` vezes e fica com
o menor corte, achando o mínimo com prob. `≥ 1 - n⁻²`; custo total `O(n⁴ log n)`.

### A6. O que melhora no algoritmo de Karger-Stein e qual sua complexidade?
As últimas contrações são as mais arriscadas. Karger-Stein recursa **duas vezes** parando em
`n' = ⌈1 + n/√2⌉` vértices (onde a prob. de o corte sobreviver ainda é `≥ 1/2`). Recorrência
`T_n = 2T(n/√2) + O(n²)`, que por **Akra-Bazzi** (`2·(1/√2)^p = 1 → p=2`) dá
**`Θ(n² log n)`**, achando o corte mínimo com prob. `Ω(1/log n)`.

### A7. Por que o teste de primalidade por divisão por tentativa é ruim?
Testa divisores até `√n`. O tamanho da entrada é `t = log n` bits, então o número de iterações
é `Θ(√n) = Θ(2^{t/2})` → **exponencial no tamanho da entrada**, mesmo com cada divisão `O(1)`.

### A8. O que é um número de Carmichael e por que ele quebra o teste de Fermat?
É um número **composto** que satisfaz `a^{n-1} ≡ 1 (mod n)` para **toda** base `a` coprima com
`n` (pseudo-primo para todas as bases). Ex.: 561, 1105, 1729. O teste de Fermat sempre responde
"primo" para eles, e existem **infinitos** (`C(n) > n^{2/7}`), então Fermat não tem garantia.

### A9. Como o teste de Miller-Rabin melhora o de Fermat? Qual sua taxa de erro?
Usa o **teorema da raiz modular** (`x² ≡ 1 → x ≡ ±1 mod p` para `p` primo). Escreve
`n-1 = 2^t·u` e testa o critério de **pseudo-primo forte**: `a^u ≡ 1` ou existe `i ∈ [0,t-1]`
com `a^{2^i u} ≡ -1`. Teorema de Monier/Rabin: se `n` é composto, **> 3/4** das bases o
denunciam, então com `k` testes `Pr(errar) ≤ (1/4)^k = 2^{-2k}`. É **co-RP** (só erra dizendo
"primo").

### A10. Por que a probabilidade de um número aleatório grande ser primo é razoável?
Pelo **teorema dos números primos** (Hadamard/Vallée Poussin), `π(n) ~ n/ln n`, logo a
probabilidade de um inteiro aleatório em `[2,n]` ser primo é `~ 1/ln n`. Para números de
centenas de bits isso ainda é alto o suficiente para sortear até achar um primo (testando
primalidade com Miller-Rabin).

---

<a name="parte-b"></a>
# Parte B: Classes de complexidade (P, NP, PP, BPP, RP, ...)

### B1. Defina a classe R(α, β).
`L ∈ R(α, β)` se existe algoritmo de decisão `A` em **tempo polinomial** tal que: se `x ∈ L`
então `Pr(A(x)="sim") ≥ α`; se `x ∉ L` então `Pr(A(x)="não") ≥ β`. A probabilidade é sobre os
bits aleatórios.

### B2. Defina RP, co-RP, ZPP, BPP e PP em termos de R(α, β).
- **RP** `= R(1/2, 1)`: erro unilateral no lado "sim".
- **co-RP** `= R(1, 1/2)`: erro unilateral no lado "não".
- **ZPP** `= RP ∩ co-RP`: erro zero (Las Vegas).
- **BPP** `= R(2/3, 2/3)`: erro bilateral **limitado** (1/3).
- **PP** `= ∪_{ε∈(0,1/2]} R(1/2+ε, 1/2+ε)`: erro bilateral **ilimitado** (pode estar perto de 1/2).

### B3. Qual a diferença essencial entre BPP e PP?
Ambas têm erro nos **dois lados**, mas em **BPP** o erro é uma constante **afastada de 1/2**
(ex. 1/3), enquanto em **PP** o `ε` pode ser arbitrariamente pequeno e até **diminuir com o
tamanho da instância**. Consequência: dá para **amplificar BPP** exponencialmente, mas **não PP**.

### B4. Por que conseguimos amplificar RP, co-RP e BPP, mas não PP?
Em RP/co-RP/BPP o erro está separado de 1/2 por uma margem **constante**, então repetir `k` vezes
e combinar (OR para RP, maioria para BPP) reduz o erro **exponencialmente** em `k` (Chernoff).
Em PP a margem `ε` pode encolher com `n`, então um número constante de repetições não garante
redução exponencial.

### B5. Dê as três caracterizações equivalentes de ZPP.
1. `ZPP = RP ∩ co-RP`.
2. Classe das linguagens **honestas** (algoritmo responde "sim"/"não"/"não sei", nunca erra ao
   responder, e `Pr("não sei") ≤ 1/2`).
3. Classe das linguagens **sem falha** (sempre responde certo em **tempo polinomial esperado** =
   Las Vegas). Repetir o honesto até não dar "não sei" mantém o tempo esperado polinomial.

### B6. Desenhe o diagrama de inclusão das classes randomizadas (Figura 4.1).
```
            PP = co-PP
           /    |     \
         NP    BQP    co-NP
           \    |     /
          BPP = co-BPP
           /          \
         RP           co-RP
           \          /
             ZPP
              |
              P
```
`P ⊆ ZPP ⊆ {RP, co-RP} ⊆ BPP ⊆ {NP, BQP, co-NP} ⊆ PP`. As setas para NP/BQP/co-NP a partir de
BPP são relações **em aberto**.

### B7. Prove (ideia) que RP ⊆ NP.
Um algoritmo RP aceita `x ∈ L` com prob. `≥ 1/2`, ou seja, existe pelo menos uma sequência
aleatória `r` que o faz responder "sim", e nunca aceita `x ∉ L`. Essa `r` serve de
**testemunha** verificável em tempo polinomial → caracterização de NP. Logo `RP ⊆ NP` (e
`co-RP ⊆ co-NP`). (Teo. 4.5)

### B8. Onde fica BQP no diagrama e por que isso é relevante?
**BQP** (bounded-error quantum polynomial) é a classe dos problemas resolvidos por algoritmos
quânticos com erro limitado. No diagrama fica **entre BPP e PP**, ao lado de NP e co-NP, com
relações com NP/co-NP **em aberto**. É a ponte com a seção de algoritmos quânticos (o algoritmo
de Deutsch é um exemplo de vantagem quântica).

### B9. O que significa "abundância de testemunhas" e por que conecta NP e RP?
NP = problemas cuja solução é **verificável** em tempo polinomial. Se, para uma instância, **mais
da metade** das candidatas são soluções válidas, sortear uma e verificar dá um algoritmo **RP**.
Esses problemas têm abundância de testemunhas; é o que torna a randomização eficaz (ex.:
identidade de polinômios).

### B10. As notas definem P e NP formalmente?
Não com o mesmo rigor das classes randomizadas. As notas assumem P/NP como pré-requisito e
caracterizam NP **informalmente** por verificação de soluções em tempo polinomial / abundância
de testemunhas (p.152). O formalismo cuidadoso é reservado a RP, co-RP, ZPP, BPP e PP via
`R(α,β)`.

---

<a name="parte-c"></a>
# Parte C: Complexidade e análise de recorrências

### C1. Enuncie o teorema de Akra-Bazzi-Leighton.
Para `T(x) = Σ_i a_i·T(b_i·x + h_i(x)) + g(x)` (com `a_i > 0`, `0 < b_i < 1`, perturbações `h_i`
pequenas e `g` polinomialmente limitada), tem-se
`T(x) = Θ( x^p·(1 + ∫₁^x g(u)/u^{p+1} du) )`, onde `p` resolve a **equação característica**
`Σ_i a_i·b_i^p = 1`.

### C2. Use Akra-Bazzi para resolver T(n) = 2T(n/√2) + O(n²).
Equação característica: `2·(1/√2)^p = 1`. Como `(1/√2)^p = 2^{-p/2}`, temos `2^{1-p/2} = 1`, logo
`p = 2`. Então `T(n) = Θ(n²·(1 + ∫₁^n cu²/u³ du)) = Θ(n²·(1 + c ln n)) = Θ(n² log n)`. (É o caso
do Karger-Stein.)

### C3. Enuncie o teorema para recorrências subtrativas (Graham et al.).
Para `T(n) = Σ_i α_i·T(n - d_i)`, seja `α` a raiz de **maior módulo** (multiplicidade `l`) do
polinômio característico `z^d - α₁z^{d-d₁} - ... - α_k z^{d-d_k}` (`d = max d_k`). Então
`T(n) = Θ(n^l·α^n)`.

### C4. O que significa uma recorrência de divisão "contrair", e por que importa?
Quando a **soma das frações** dos subproblemas é `< 1`, o trabalho por nível decai
geometricamente e o total é **linear** (dominado pela raiz). Se a soma é **exatamente 1**, não há
garantia de linearidade (pode virar `n log n` ou pior). Ex.: mediana das medianas contrai sse
`1/g + (3g-1)/(4g) < 1`, ou seja `g ≥ 5`; para `g=3` a soma é 1.

### C5. Qual a cota inferior para as operações de uma fila de prioridade e por quê?
Pelo menos uma entre `insert` e `extract-min` (deletemin) deve custar **`Ω(log n)`**. Senão,
poderíamos ordenar `n` elementos com `n` inserts + `n` deletemins em `o(n log n)`, violando a
cota inferior de ordenação por comparação.

### C6. Por que medir constantes empíricas, se a complexidade assintótica já é conhecida?
Porque dois algoritmos de **mesma classe** podem ter desempenho prático muito diferente: a
constante decide. Ex.: Quickselect e Mediana das Medianas são ambos `O(n)`, mas a Mediana das
Medianas é ~2,7× mais lenta (25,7 vs 9,7 ns/elemento). A assíntota não prevê o vencedor prático.

---

<a name="parte-d"></a>
# Parte D: Seleção do k-ésimo elemento

### D1. Quais algoritmos de seleção foram comparados e suas complexidades?
- **Seleção Ingênua** (ordena e indexa): `Θ(n log n)`, independente de `k`.
- **Quickselect aleatorizado**: `O(n)` esperado, `Θ(n²)` pior caso.
- **Mediana das Medianas** (`g ≥ 5`): `Θ(n)` pior caso (com ressalva sobre duplicatas).
- **`std::nth_element`** (introselect): `O(n)` pior caso.

### D2. Deduza a condição de linearidade da mediana das medianas em função de g.
Com grupos de tamanho `g`, a mediana das medianas garante `≈ n(g+1)/(4g)` elementos de cada lado;
o lado maior tem `≤ (3g-1)/(4g)·n` e o subproblema das medianas tem `n/g`. Recorrência:
`T(n) ≤ T(n/g) + T((3g-1)/(4g)·n) + O(n)`. Contrai (linear) sse
`1/g + (3g-1)/(4g) < 1`. Resolvendo: `4 + 3g - 1 < 4g → g > 3`, logo **`g ≥ 5`** (ímpar). Para
`g = 3` a soma é exatamente 1: **sem garantia**.

### D3. (experimento) O que os expoentes empíricos de E1 confirmaram?
Em escala log-log: Ingênua `k≈1,12` (reflete o fator `log n`), Quickselect `0,99` e Mediana das
Medianas `1,04` (lineares). A `n=5×10⁵`: Ingênua ~11,4 milhões de comparações, Mediana das
Medianas ~5,4 milhões, Quickselect só ~1,7 milhão. A razão acessos/comparações ~3,1 do
Quickselect é a assinatura da partição de Lomuto (cada troca custa 4 acessos).

### D4. (experimento) Por que a Mediana das Medianas é mais lenta que o Quickselect se ambos são O(n)?
A **constante** é maior: ~25,7 ns/elemento contra 9,7 do Quickselect (fator ~2,7×). O custo extra
vem de **ordenar cada grupo** para achar sua mediana e da **recursão aninhada** de `get_pivot`.
Mesma classe assintótica, constante pior.

### D5. (experimento) O que E3 revelou sobre a distribuição do tempo do Quickselect?
Média e desvio-padrão crescem ambos **linearmente** em `n`, logo a **variância ∝ n²** e o
**coeficiente de variação CV = σ/μ ≈ 0,28 é constante** (a variabilidade relativa não cresce com
a escala). A distribuição é levemente assimétrica à direita (0,6 a 0,8), por sequências
ocasionais de pivôs ruins, mas sem agravamento relativo com `n`.

### D6. (experimento) Como cada algoritmo depende do posto k (E4)?
Ingênua é **independente de `k`** (ordena tudo). Mediana das Medianas é **quase plana** (divisão
garantida ~30/70). Quickselect tem dependência **moderada**, mais barato nos extremos
(`k/n ≈ 0,99`) e com pico em postos interiores.

### D7. (experimento) Descreva a forma de U da varredura de g (E5) e os ótimos.
Custo vs `g` tem forma de **U**: `g` pequeno → muitos níveis de recursão (mais comparações);
`g` grande → ordenar cada grupo fica caro. Ótimo empírico: **comparações em `g=7`**, **tempo em
`g=9`**. A profundidade de recursão cai monotonicamente com `g`. `g=5` é competitivo mas não
ótimo nesta máquina.

### D8. (experimento) Por que g=3 permanece linear na prática (E6) apesar da teoria?
A recorrência de `g=3` (`T(n/3) + T(2n/3) + O(n)`, soma das frações = 1) **não contrai**, então
**no pior caso** pode degradar. Mas em entradas **benignas** (uniforme, ordenada) a mediana das
medianas escolhe pivôs muito melhores que a garantia, dando divisões quase equilibradas; logo
`g=3` fica linear até `n=10⁶` (expoente 1,04). **A degradação é estritamente de pior caso.**

### D9. (experimento, ACHADO PRINCIPAL) O que acontece com entradas de chaves repetidas (E7)?
Com `alleq` (todos iguais), a **partição de Lomuto de duas vias** manda todos os iguais para um
lado, removendo só 1 elemento por nível:
- **Quickselect → `Θ(n²)`** (expoente 2,00).
- **Mediana das Medianas → expoente 3,85** (pior que cúbico), pela recursão aninhada de
  `get_pivot`; chegou a estourar o timeout de 20 s.
- **`std::nth_element` resiste** (expoente 0,99).

A garantia linear da literatura **pressupõe partição de três vias** que isole as chaves iguais ao
pivô; a implementação de duas vias **não preserva** essa garantia. (Reportado como achado, de
propósito, em vez de mascarado.)

### D10. (experimento) Por que std::nth_element não usa nenhum dos três algoritmos puros?
Ele implementa **introselect**: Quickselect com pivô **mediana-de-3** que monitora a profundidade
de recursão e, ao passar de `~2 log₂ n`, **recua para heapselect** (pior caso garantido). Assim
combina a velocidade média do Quickselect com robustez: fica **linear (≈2n a 3n comparações) em
todas as distribuições**, inclusive `alleq` e `dup_2`, onde Quickselect e Mediana das Medianas
explodem (6 milhões e 325 milhões de comparações).

### D11. (experimento) Qual a recomendação prática final (E8)?
Para entrada aleatória **não há vencedor absoluto entre Quickselect e `nth_element`** (empatados,
lineares), ambos dominando a Mediana das Medianas e a Ingênua. Ordem:
`nth_element ≈ Quickselect < Mediana das Medianas < Ingênua`. A Ingênua só se justifica se o
vetor ordenado também for necessário; a Mediana das Medianas como garantia de pior caso **se** a
partição tratar duplicatas. No geral, `std::nth_element` é a escolha recomendada.

---

<a name="parte-e"></a>
# Parte E: Dijkstra com heap k-ário

### E1. Qual a complexidade total de Dijkstra com heap k-ário?
`O(n) + n·deletemin + n·insert + m·update`. Com o heap `k`-ário (`insert`/`decrease-key` =
`O(log_k n)`, `extract-min` = `O(k·log_k n)`), o total é
**`O(n·k·log_k n + m·log_k n)`**.

### E2. Por que sift_down custa O(k·log_k n) e sift_up custa O(log_k n)?
`sift_up` sobe comparando o nó só com o **pai** (1 comparação por nível), e a altura é `log_k n`,
logo `O(log_k n)`. `sift_down` desce comparando, em cada nível, com os **k filhos** para achar o
menor, logo `O(k)` por nível × `log_k n` níveis = **`O(k·log_k n)`**.

### E3. Qual o papel do array `pos_` na implementação?
Guarda a **posição de cada vértice dentro do heap**, permitindo localizar um vértice em **`O(1)`**
para a operação `decrease_key` (sem ele, seria necessário varrer o heap). É atualizado a cada
troca em `sift_up`/`sift_down`.

### E4. Enuncie a proposição de Noshita (caso médio de Dijkstra).
Com pesos aleatórios atribuídos aos arcos de entrada, o número médio de operações `update` é
**`n·ln(m/n)`**. Razão: uma aresta causa `update` só se for um **mínimo local** na sequência
ordenada; o número esperado de mínimos locais numa permutação aleatória é `H_k - 1 ≤ ln k`.

### E5. (experimento) Qual o grau k ótimo do heap e por quê?
**`k = 8`**, especialmente em grafos densos. Trade-off: `k` pequeno → árvore profunda (`sift_up`
caro); `k` grande → árvore larga (`sift_down`/Insert caros). Grafos densos fazem mais `update`
(que usa `sift_up`), então se beneficiam de `k` maior; mas acima de `k=16` o custo de
`sift_down` supera a economia.

### E6. (experimento) Como foi validado o escalonamento assintótico?
Normalizando o tempo por `T / ((n+m)·log n)`. As curvas resultantes ficam praticamente
**horizontais** mesmo com o grafo crescendo, confirmando que a implementação escala conforme a
complexidade teórica `O(m log_k n + n·k·log_k n)`.

### E7. (experimento) O que o teste no grafo real USA.gr (DIMACS) revelou?
Com ~24 milhões de vértices, o algoritmo escalou corretamente, mas o **gargalo de desempenho é a
E/S de disco** (leitura do arquivo para a memória), não o cômputo do caminho mais curto. O
consumo de RAM ficou estável.

### E8. Qual a diferença entre o algoritmo de Dijkstra e a busca A*?
A* usa uma **heurística** `h(v)` (estimativa da distância ao destino) e processa vértices por
`f(v) = g(v) + h(v)`. Se `h` é **admissível** (`h(v) ≤ dist(v,t)`), A* devolve o ótimo; se `h` é
**consistente**, processa cada vértice só uma vez (como Dijkstra). Dijkstra é o caso `h ≡ 0`.

---

<a name="parte-f"></a>
# Parte F: Fluxo máximo

### F1. O que é o grafo residual e como ele permite aumentar o fluxo?
`G_f` tem, para cada arco, um arco **forward** com capacidade `c_a - f_a` (folga) e um arco
**backward** com capacidade `f_a` (permite **desfazer** fluxo). Um caminho `s`-`t` em `G_f`
(caminho aumentante) com gargalo `g` permite aumentar o fluxo em `g`, mantendo a conservação.

### F2. Enuncie e explique o teorema fluxo máximo = corte mínimo.
O valor do **fluxo máximo** `s`-`t` é igual ao valor do **corte** `s`-`t` **mínimo** (Teo. 1.10).
Todo corte é limite superior do fluxo (`f(s) = f(X) - f(X̄) ≤ c(X,X̄)`); quando Ford-Fulkerson
para, o conjunto `X` alcançável a partir de `s` em `G_f` define um corte cujos arcos estão
**saturados**, igualando fluxo e corte.

### F3. Por que Ford-Fulkerson é só pseudo-polinomial e como Edmonds-Karp resolve?
FF executa no máximo `C` (= fluxo máximo) iterações, cada uma `O(n+m)`, então `O((n+m)C)`; `C`
pode ser exponencial no tamanho da entrada (e com capacidades irracionais FF pode nem terminar).
**Edmonds-Karp** usa **BFS** (caminho mais curto); a distância `s`-`v` em `G_f` cresce
monotonicamente, dando `O(nm)` iterações e tempo **`O(nm²)`** (fortemente polinomial).

### F4. Compare os seis algoritmos de fluxo e suas complexidades.
| Algoritmo | Complexidade | Busca |
|---|---|---|
| Ford-Fulkerson DFS (FF) | `O(mC)` | DFS |
| FF randomizado (RDFS) | `O(mC)` | DFS + embaralhamento |
| Edmonds-Karp (EK) | `O(nm²)` | BFS |
| Dinitz (Di) | `O(n²m)` | grafo de níveis + fluxo bloqueante |
| Escalonamento de capacidade (EC) | `O(m² log C)` | BFS restrita a arcos `≥ Δ` |
| Caminho mais gordo / fattest (FP) | `O(m² log C)` | Dijkstra modificado |

### F5. Defina as três métricas de "defeito" do relatório de fluxos.
- **`r = F/F̄`**: razão entre fases reais `F` e o limite teórico `F̄` (mede quão frouxo é o
  limite).
- **`s̄`**: fração média de **vértices** tocados por fase.
- **`t̄`**: fração média de **arcos** avaliados por fase.

### F6. (experimento) Quão frouxos são os limites teóricos de fases?
Muito frouxos: `r` fica entre `~10⁻⁵` e `~10⁻²`. Ex.: Edmonds-Karp com `F̄ = nm/2 ≈ 4×10⁸`
executa apenas ~4700 fases (5 ordens de magnitude de folga). Conclusão: os limites de pior caso
**não preveem** o desempenho prático.

### F7. (experimento) Por que Dinitz foi o melhor algoritmo na prática?
Faz **pouquíssimas fases**: na família `BasicLine` satura a rede em **1 única fase**; na
`DoubleExpLine`, `F` cresce só **logaritmicamente** com `n` (8 a 18 enquanto `n` vai de 1002 a
20002), contra `F̄ = n`. Combina número mínimo de fases com custo por fase controlado, dando a
curva de tempo mais baixa.

### F8. (experimento) Por que Dinitz é o único com t̄ > 1?
`t̄ > 1` significa que, em média, **cada arco é avaliado mais de uma vez por fase**. Dinitz faz
**múltiplas iterações de DFS** sobre o grafo de níveis em cada fase (para construir o fluxo
bloqueante), resultando em várias travessias por fase (`t̄` de 2,8 a 4,1). EK toca quase todos os
arcos uma vez (`t̄ ≈ 0,97`); RDFS toca pouquíssimos (`t̄ ≈ 0,016`).

### F9. (experimento) Por que o Fattest Path teve desempenho inferior ao esperado?
Apesar de ter limite de fases `F̄ = m log C` mais preciso que EK, paga um **custo alto por fase**
pela manutenção do **Max-Heap** (sobrecarga de estrutura de dados, péssima localidade), ficando
mais lento que o Escalonamento de Capacidade mesmo executando o **mesmo número de fases**.

### F10. (experimento) Qual a "tensão fundamental" entre as métricas de fluxo?
Algoritmos com **poucas fases** tendem a tocar **muitos arcos por fase** (EK, Dinitz, `t̄` alto);
algoritmos com **muitas fases** tocam **poucos arcos por fase** (RDFS, `t̄` baixo). O ótimo
prático (Dinitz) está em minimizar fases mantendo o custo por fase sob controle.

---

<a name="parte-g"></a>
# Parte G: Emparelhamentos bipartidos

### G1. O que é um caminho M-aumentante e enuncie o teorema de Berge.
Um caminho **alternante** (alterna arestas fora e dentro de `M`) que começa e termina em
vértices **livres**. **Teorema de Berge:** um emparelhamento `M` é **máximo** sse **não existe
caminho M-aumentante**. Aumentar = fazer a diferença simétrica `M ⊕ P` (aumenta `|M|` em 1).

### G2. Como funciona o algoritmo de Hopcroft-Karp e por que bastam O(√n) fases?
Agrupa aumentos em **fases**: cada fase faz uma **BFS** (rede em camadas) + uma **DFS** que acha
um conjunto **máximo de caminhos aumentantes mínimos vertex-disjuntos**, todos em `O(m)`.
Bastam `O(√n)` fases porque (Lema 1.29) o **comprimento mínimo** de caminho aumentante cresce
`≥ 2` por fase e (Lema 1.30) após `√n` fases o emparelhamento está a `≤ √n` do máximo. Total
**`O(m√n)`**.

### G3. Enuncie o teorema de König.
Em grafos **bipartidos**, o tamanho do **emparelhamento máximo** é igual ao tamanho da **menor
cobertura de vértices** (Teo. 1.16, Berge 1951 / Egerváry no caso ponderado). Dual: o conjunto
independente máximo é o complemento da cobertura mínima.

### G4. Como o caso ponderado lida com pesos negativos (BF vs JD)?
Busca o caminho aumentante de **menor custo** (custo negado) na rede residual.
- **Bellman-Ford (BF)** suporta pesos negativos diretamente (`O(nm)` por aumento → `O(n²m)`).
- **Johnson/Dijkstra (JD)** usa **potenciais** que transformam os pesos em **não-negativos** após
  cada aumento (`d'_{uv} ≥ 0`), permitindo Dijkstra com heap (`O(m log n)` por aumento →
  `O(nm log n)`).

### G5. Dê as complexidades dos quatro algoritmos de emparelhamento em função de α (m = n^α).
| Algoritmo | Complexidade | Expoente |
|---|---|---|
| Simple | `O(n^{1+α})` | `1+α` |
| Hopcroft-Karp | `O(n^{0,5+α})` | `0,5+α` |
| Húngaro + BF | `O(n^{2+α})` | `2+α` |
| Húngaro + JD | `O(n^{1+α} log n)` | `1+α (+log)` |

### G6. (experimento) Por que o número de fases do HK ficou constante (U1)?
Para `α ≥ 1,5` (grafos densos gerados), o HK acha o emparelhamento perfeito em **1 a 2 fases**
(expoente empírico ~0), **não `O(√n)`**. Uma única BFS+DFS já satura. Para `α=1,0` (esparso) as
fases crescem devagar (expoente 0,16). Logo a complexidade efetiva em grafos densos é
`O(m) = O(n²)`, não `O(n^{2,5})`.

### G7. (experimento) Quando o algoritmo Simple é melhor que Hopcroft-Karp?
Para **grafos esparsos** (`α = 1,0`): o Simple é **~5× mais rápido** em toda a faixa testada (até
`n=10000`). O **overhead** de inicializar o array `dist` e montar a rede em camadas do HK não
compensa quando os caminhos aumentantes já são curtos. Para `α ≥ 1,5` o HK domina (até ~1084× em
`α=2,0`, `n=10000`).

### G8. (experimento, CONTRAINTUITIVO) Por que BF supera JD em grafos densos (W3/C2)?
Para `α = 2,0`, **BF é ~10× mais rápido** que JD (vantagem cresce com `n`), apesar de JD ter
melhor assíntota. Dois motivos:
1. **Terminação antecipada** do BF (flag `relaxed`) derruba o expoente empírico de 4,0 para ~3,0,
   e o acesso sequencial à matriz tem ótima **localidade de cache**.
2. O **heap do JD** com `O(n²)` entradas tem alto custo constante e péssima localidade.
A vantagem de JD é máxima em `α ≈ 1,25` e o ponto de cruzamento fica em `α ≈ 1,75`.

### G9. (experimento) Que evidência mostra que os potenciais de Johnson foram mantidos corretamente?
BF e JD produziram valores de emparelhamento **idênticos** em todas as instâncias (W1, W4),
**inclusive no regime totalmente negativo** (o mais tensionado). Uma única violação de
`d'_{uv} ≥ 0` faria o Dijkstra calcular um caminho mínimo errado e JD divergir de BF; a
concordância exata (mais a validação contra o oráculo de força bruta em W0) é evidência indireta
de que a invariância foi preservada.

---

<a name="parte-h"></a>
# Parte H: Algoritmos quânticos

### H1. Enuncie os quatro postulados da mecânica quântica.
1. **Estados:** vetores unitários de um espaço de Hilbert `H` (complexo, com produto interno).
2. **Evolução:** por operador **unitário** `U` (equação de Schrödinger, `H` hermitiano).
3. **Medição (Born):** operadores `{M_m}` com `Σ M_m†M_m = I`; `P(m) = ⟨ψ|M_m†M_m|ψ⟩`; estado
   colapsa para `M_m|ψ⟩/√P(m)`.
4. **Composição:** produto tensorial `H₁ ⊗ ... ⊗ H_n`, estado `|ψ₁⟩ ⊗ ... ⊗ |ψ_n⟩`.

### H2. O que é um qubit e qual a condição de normalização?
Sistema de dimensão `N=2`, estado `|ψ⟩ = α₀|0⟩ + α₁|1⟩` com `α_i ∈ ℂ`. Normalização (postulado
1): **`|α₀|² + |α₁|² = 1`**. Ao medir, observa-se `|i⟩` com probabilidade `|α_i|²`.

### H3. O que é a esfera de Bloch e por que um qubit tem só 2 parâmetros reais?
Embora `α₀, α₁` deem 4 parâmetros reais, a normalização tira 1 e a **fase global** é
fisicamente irrelevante (`|zα|² = |α|²` se `|z|=1`), tirando outro. Sobram 2:
`|ψ⟩ = cos(θ/2)|0⟩ + sin(θ/2)e^{iφ}|1⟩`, um **ponto na esfera de Bloch** (`θ` polar a partir de
`|0⟩`, `φ` azimutal).

### H4. Escreva as matrizes de Pauli e a porta de Hadamard. O que faz cada uma?
`I = [[1,0],[0,1]]`; `X = [[0,1],[1,0]]` (**NOT**: `X|i⟩=|1-i⟩`); `Y = [[0,-i],[i,0]]`;
`Z = [[1,0],[0,-1]]` (flip de fase em `|1⟩`). **Hadamard** `H = (1/√2)[[1,1],[1,-1]]` cria
superposição: `H|0⟩ = |+⟩`, `H|1⟩ = |-⟩`, e `H² = I`.

### H5. O que é uma porta controlada? Descreva o CNOT.
Uma porta cujo qubit de **controle** decide se o operador age no qubit **alvo**. O **CNOT** (NOT
controlado) faz `|a⟩|b⟩ → |a⟩|a⊕b⟩`: inverte o alvo sse o controle for `|1⟩`. Notação: ponto
preto no controle, ⊕ no alvo.

### H6. Calcule (X ⊗ Z)|01⟩.
`(X⊗Z)|01⟩ = X|0⟩ ⊗ Z|1⟩ = |1⟩ ⊗ (-|1⟩) = -|11⟩`.

### H7. Qual problema o algoritmo de Deutsch resolve e qual a vantagem quântica?
Dada `f:{0,1}→{0,1}` por oráculo, descobrir `f(0) ⊕ f(1)` (se `f` é **constante** ou
**balanceada**). **Classicamente** precisa de **2 consultas**; **quanticamente, 1**. É o primeiro
exemplo de **vantagem quântica**.

### H8. Descreva o circuito de Deutsch e o resultado da medição.
Prepara `|0⟩|1⟩`, aplica **Hadamard** nos dois qubits, depois o oráculo `U_f|x⟩|y⟩ =
|x⟩|y⊕f(x)⟩`, depois Hadamard no primeiro qubit e mede. Mede-se **0 se `f(0)=f(1)`** (constante)
e **1 se `f(0)≠f(1)`** (balanceada). O mecanismo é o **phase kickback** (`|v⟩-|1⊕v⟩ =
(-1)^v|-⟩`), que converte `(-1)^{f(0)} ± (-1)^{f(1)}` num resultado mensurável por interferência.

### H9. A que classe de complexidade os algoritmos quânticos se ligam?
**BQP** (bounded-error quantum polynomial), que no diagrama das notas fica **entre BPP e PP**, ao
lado de NP e co-NP (relações em aberto).

---

<a name="parte-i"></a>
# Parte I: Questões integradoras (dissertativas)

### I1. "Limites de pior caso são frequentemente frouxos." Justifique com dois experimentos.
- **Fluxos:** `r = F/F̄` ficou em `~10⁻⁵` (EK executa 4700 fases contra `F̄ ≈ 4×10⁸`).
- **Emparelhamentos:** o HK roda `O(1)` fases em grafos densos, não as `O(√n)` do pior caso.
Conclusão: limites de pior caso dão **garantias**, não **previsões** de caso médio; por isso a
avaliação experimental é parte da engenharia de algoritmos.

### I2. Como detalhes de implementação revertem previsões assintóticas? Dê exemplos.
- **BF vs JD (denso):** BF, com pior assíntota (`O(n^{2+α})`), **vence** JD para `α=2,0` por
  ~10×, graças à terminação antecipada e à localidade de cache; o heap de JD é caro.
- **Fattest Path:** limite de fases melhor, mas o Max-Heap o deixa mais lento que o Escalonamento
  de Capacidade.
- **Mediana das Medianas:** `O(n)` na teoria, mas constante ~2,7× pior que o Quickselect.
Moral: localidade de cache, overhead de estruturas de dados e terminação antecipada importam na
faixa prática de `n`.

### I3. Dê um caso em que uma garantia teórica depende de uma hipótese silenciosa.
A **mediana das medianas** é `Θ(n)` no pior caso **somente** se a partição isolar as chaves
**iguais** ao pivô (partição de **três vias**). Com partição de **Lomuto (duas vias)**, sob
`alleq` ela degrada para expoente **3,85** (pior que cúbico), porque cada nível remove só 1
elemento e há recursão aninhada. A garantia da literatura assumia a partição de três vias.

### I4. Como a aleatoriedade traz robustez? Compare dois algoritmos.
- **RDFS (fluxos):** o embaralhamento das arestas imuniza contra ordens **adversárias** e produz
  caminhos curtos (`t̄ ≈ 0,016`, tocando pouquíssimos arcos).
- **Quickselect:** o pivô **aleatório** o torna imune a arranjos fixos (ordenado, organpipe); só
  **duplicatas** (não a ordem) o derrubam.
Em ambos, a aleatoriedade troca dependência da entrada por uma garantia probabilística.

### I5. Relacione um algoritmo randomizado do cap. 4 com sua classe de complexidade.
- **Miller-Rabin** → **co-RP** (só erra dizendo "primo"; `Pr(erro) ≤ 4^{-k}`).
- **Teste de identidade de polinômios** → **co-RP** (erro só no lado "sim").
- **Quickselect** → **Las Vegas** (sempre correto; tempo esperado `O(n)`), análogo a **ZPP** na
  ótica de classes.
- **Karger** → Monte Carlo (acerta o corte mínimo com prob. `Ω(1/n²)`, amplificável).

### I6. Por que a randomização é importante para a classe NP (abundância de testemunhas)?
Se um problema de NP tem, para cada instância, **mais da metade** das candidatas sendo
testemunhas válidas, então **sortear** uma e **verificar** dá um algoritmo **RP** eficiente. Isso
mostra que problemas com abundância de testemunhas são "fáceis" via randomização, ilustrando o
poder prático dos algoritmos randomizados (ex.: identidade de polinômios).

---

<a name="parte-j"></a>
# Parte J: Questões de provas anteriores (com gabarito)

> Questões reais das provas do Prof. Marcus Ritt (pasta `Estudo/provas-antigas`), com as
> **respostas oficiais** resumidas. A prova de **2025/1** é a mais alinhada com os tópicos
> atuais; as mais antigas (2017/1) incluem temas de **aproximação** que podem ou não cair.
> Padrão recorrente das provas: "**Prova ou contra-exemplo**" (afirmação de um colega, julgue) e
> "**liste/justifique complexidades**".

## Prova 2025/1 (a mais relevante)

### J1. (Complexidades) Liste e explique as complexidades de Dijkstra, Edmonds-Karp, Caminho mais gordo, Hopcroft-Karp e Algoritmo Húngaro.
Resposta oficial:
- **Dijkstra:** `O((n+m) log n)` com heap binário: `n` inserts + `n` deletemins + `m` updates,
  cada um `O(log n)`.
- **Edmonds-Karp:** `O(nm²)`: no máximo `nm` fases, cada fase uma BFS de custo `O(n+m)`.
- **Caminho mais gordo:** `O(Dm log C)`: no máximo `m log C` iterações (`C` = limite superior do
  fluxo), cada iteração aplica Dijkstra em tempo `D = O(n log n + m)`.
- **Hopcroft-Karp:** `O(n^{5/2})`: no máximo `√n` fases, cada uma uma BFS `O(n+m)`, logo
  `O((n+m)√n) = O(m√n) = O(n^{5/2})` (no caso denso).
- **Algoritmo Húngaro:** `O(nD)`: no máximo `O(n)` fases (aumentações), cada uma um Dijkstra em
  tempo `D`.

### J2. (Caminhos curtos com distâncias negativas) (a) Sob quais condições Bellman-Ford e Johnson dão resultados corretos? (b) Explique a ideia de potenciais no Johnson.
(a) **Quando não existem ciclos de custo negativo.**
(b) Um **potencial** é uma função `p: V → ℝ` tal que `d_uv ≥ p_v - p_u` para todo arco. Então os
custos transformados `d'_uv = d_uv - (p_v - p_u) ≥ 0` são **não-negativos**. A propriedade-chave:
um caminho `s`-`v` mais curto em `d` também é mais curto em `d'`; logo, de posse de um potencial,
pode-se rodar um algoritmo de caminhos mais curtos para **distâncias não-negativas** (Dijkstra,
mais eficiente).

### J3. (Fluxo máximo, MUITO cobrado) Enuncie o teorema fluxo máximo = corte mínimo e discuta multiplicidade de fluxos e cortes.
(a) **Teorema:** para qualquer corte `X` e fluxo `f`, `f(s) ≤ c(X)`; se `X*` é corte mínimo e
`f*` fluxo máximo, então `f*(s) = c(X*)`. O valor do fluxo máximo iguala o do corte mínimo.
(b) **Vários fluxos máximos, um único corte mínimo: possível.** Ex.: grafo "diamante"
`s → {a,b} → c → t` com capacidades unitárias tem 2 fluxos máximos (ou infinitos reais), ambos
de valor 1, mas um só corte mínimo.
(c) **Um fluxo máximo, vários cortes mínimos: possível.** Ex.: caminho `s → · → t` com
capacidades unitárias tem fluxo máximo único (valor 1), mas **dois cortes mínimos** (cada aresta
é um corte).
(d) **(Bônus) Qual corte mínimo os algoritmos do tipo Ford-Fulkerson devolvem?** O corte mínimo
**`C_min`**, definido por `X` = conjunto de vértices **alcançáveis a partir de `s` no grafo
residual** quando o algoritmo para. Esse é o **menor** corte mínimo (`C_min = ∩_{C} C` sobre
todos os cortes mínimos): como a busca é a partir de `s`, não há arco saindo de `X`, e
`C_min ⊆ C` para todo corte mínimo `C`.

### J4. (Algoritmos randomizados) Algoritmo Las Vegas polinomial cuja resposta é invertida com prob. p=0.1; reduza o erro para < 0.03.
Resposta oficial: aplicação de **amplificação**. Rode o algoritmo **3 vezes** e devolva a resposta
**majoritária**. A probabilidade de falhar (maioria errada) é
`3·(0.1)²·(0.9) + (0.1)³ = 0.027 + 0.001 = 0.028 ≤ 0.03`.

### J5. (Heaps, min heaps) Para cada ideia, prove ou dê contra-exemplo.
(a) **deletemin** trocando a raiz por `∞`, fazendo heapify-down e removendo a folha resultante:
**não funciona.** O `∞` vira *uma* folha, mas não necessariamente a **última** folha; remover
uma folha arbitrária desbalanceia o heap (perde a forma de árvore completa e as garantias de
complexidade). Contra-exemplo: raiz 3 com filhos 4 e 5.
(b) Trocar **irmãos** (com suas subárvores), `l(p) := r(p)` e `r(p) := l(p)`: **funciona.** A
propriedade de heap (pai ≤ filhos) é preservada, pois só reordena filhos do mesmo pai.
(c) Permutar **todas as folhas**: **não funciona.** Contra-exemplo: as folhas trocadas podem
violar a propriedade em relação a seus pais (ex.: folhas `6 6 1 1` sob pais menores).

### J6. (Caminhos curtos / barreira de ordenação) Dijkstra também ordena os vértices por distância, logo qualquer algoritmo assim é Ω(n log n). Certo?
**Está certo.** Guardar os vértices na ordem de processamento custa `O(1)` por vértice (sem custo
extra efetivo), então Dijkstra de fato **ordena** os vértices por distância. Como qualquer
algoritmo baseado só em **comparações** precisa de `Ω(n log n)` para ordenar, **não pode existir**
um algoritmo de caminhos mais curtos mais rápido que `n log n` que *também* produza os vértices em
ordem de distância (senão ordenaríamos `a_1,...,a_n` criando `G = ([0,n], {0i})` com `d_{0i}=a_i`
mais rápido). Observação: existem algoritmos mais rápidos que produzem **só** as distâncias, sem
ordenar (Duan et al., 2025).

## Prova 2022/2

### J7. (Corte mínimo) Existe o arco uv. Sobre o valor C do corte mínimo uv: (a) C ≥ c_uv, (b) C = c_uv, (c) C ≤ c_uv?
**(a) é verdadeira: `C ≥ c_uv`.** Como o corte mínimo `uv` **separa** `u` de `v`, o arco `uv`
tem obrigatoriamente que fazer parte do corte, então o peso do corte é pelo menos `c_uv`.

### J8. (Caminhos curtos) Duas SCCs U e W ligadas por um único arco uv; recompute em tempo linear as distâncias c'_v de s∈U após mudar d_uv para d'_uv.
Para `v ∈ U`: o caminho mais curto `s`-`v` **não passa** por `uv` (senão U e W seriam uma só SCC),
logo `c'_v = c_v` (inalterado). Para `v ∈ W`: **todo** caminho mais curto passa por `uv` (único
arco que liga U a W), logo `c'_v = c_v + (d'_uv - d_uv)`. Atualização em tempo linear.

### J9. (Miller-Rabin) Execute para p=89, base a=50.
Primeiro testa `(89, 50) = 1` por Euclides: `(89,50)=(50,49)=(49,1)=1`, passa. Escreve
`89-1 = 88 = u·2^t` com `u=11`, `t=3`. Calcula `50^{11} mod 89`: usando `2^{11} ≡ 1` e
`5^4 ≡ 2`, obtém-se `50^{11} ≡ 2^{11}·5^{22} ≡ 2^5·5^2 ≡ 88 ≡ -1 (mod 89)`. Como deu `-1`, o
algoritmo responde (corretamente) **"sim"** (provável primo).

### J10. (Fluxo máximo) (a) Caminho st mais curto P ⊆ F (suporte do fluxo máximo)? (b) P arbitrário e corte mínimo C: necessariamente P ∩ C ≠ ∅?
(a) **Não.** Contra-exemplo: o caminho mais curto pode usar arcos fora do suporte do fluxo (ex.:
triângulo onde o fluxo vai por `s→u→t` mas o caminho mais curto é `s→v→...`).
(b) **Sim.** Por definição, um corte `C` separa `s` de `t`, então **qualquer** caminho `st` tem
que cruzar `C` (interseção não vazia).

### J11. (Emparelhamentos) Reduza o "minimum bottleneck matching" (emparelhamento perfeito que minimiza a maior aresta) ao emparelhamento perfeito.
**Busca binária** no maior custo de aresta permitido. Para cada custo candidato, mantém só as
arestas com custo `≤` candidato e testa se existe **emparelhamento perfeito**. Se sim, desce
(tenta limite menor); se não, sobe.

### J12. (Árvores de Gomory-Hu) O que é e qual algoritmo polinomial a constrói?
Uma árvore `H` que **codifica todos os cortes mínimos**: o corte mínimo `uv` é a **aresta de
menor peso** no único caminho entre `u` e `v` em `H`; removê-la dá os dois lados do corte.
**Algoritmo:** começa com um super-vértice representando todos; repetidamente escolhe um
super-vértice com mais de um vértice original, escolhe dois representados `u`, `v`, computa o
corte mínimo `uv` e separa o super-vértice de acordo, até ter `n` vértices. Faz `n-1` cálculos de
corte mínimo, logo é polinomial.

## Prova 2021/1

### J13. (Emparelhamentos) Dado M (|M|=2) e M* perfeito, exiba 3 caminhos M-aumentantes disjuntos em M ⊕ M*.
Como `M ∩ M* = ∅`, `M ⊕ M* = M ∪ M*`; o grafo se decompõe em **três caminhos aumentantes**
disjuntos. (Na prova: os caminhos `1509`, `48`, `3627`, lendo os vértices ao longo de cada
componente.) Pelo Teo. de Hopcroft-Karp, `M ⊕ M*` contém `≥ |M*| - |M| = 3` caminhos
M-aumentantes vertex-disjuntos.

### J14. (Randomizados) Las Vegas com resposta invertida com prob. p=0.1: qual a menor classe (por inclusão) a que pertence?
**BPP.** Como o erro é **bilateral** (qualquer resposta pode ser invertida) mas **limitado** (0.1,
longe de 1/2), o algoritmo pertence a BPP (não a RP/co-RP, que exigem erro unilateral).

### J15. (Caminhos curtos) Após mudar o peso de uma aresta e, quando rodar Dijkstra com as distâncias antigas de G produz as corretas em G'?
- **Se o peso aumenta:** **não** necessariamente. As distâncias antigas podem ficar erradas.
  Contra-exemplo: `G=({a,b},{ab})`, `d_ab=1 → d'_ab=2`. Dijkstra relaxa repetidamente, e partir de
  distâncias antigas subestimadas não corrige.
- **Se o peso diminui:** **correto**, desde que o Dijkstra seja a versão que **insere todos os
  vértices** (inicialmente ou ao descobrir). Sem isso, falha (contra-exemplo
  `G=({a,b,c},{ab,bc})`, `d_bc=2 → d'_bc=1`).

### J16. (Fluxos) Dado um algoritmo de fluxo máximo s-t, como achar uma circulação? E fluxo máximo entre conjuntos S (origens) e D (destinos)?
**Circulação:** sempre existe a **circulação trivial 0**. Para conjuntos, introduz **super-vértices**
auxiliares: uma super-origem `s'` ligada a todo `s ∈ S` e um super-destino `t'` ligado a todo
`d ∈ D`, reduz a um problema `s'`-`t'`, e depois remove os vértices artificiais.

### J17. (Miller-Rabin) Mostre que para n par (n=2i, i≥1) o teste responde "sim" com prob. ≤ 0.5.
Com bases sorteadas em `[1, n-1]` e `n` par, há `n/2 - 1` bases **pares** (que não são coprimas
com `n`); para elas `(a,n) ≠ 1` e o algoritmo responde **"não"** (composto). Logo a probabilidade
de responder erradamente "sim" é `≤ (n/2 + 1)/(n - 1) = 1/2 + 3/(2(n-1))`, que tende a 1/2.

## Prova 2017/1 (inclui tópicos de aproximação)

### J18. (Fluxo máximo) Ignorar os arcos backward no Edmonds-Karp dá uma 2-aproximação?
**Não** (contra-exemplo). Sem os arcos para trás, o algoritmo não consegue redirecionar fluxo e
pode parar muito abaixo do máximo. Contra-exemplo com capacidades unitárias: um caminho escolhido
cedo "bloqueia" outros e o fluxo encontrado fica em 1 quando o máximo é 3 (bem menos que metade).
Os **arcos backward são essenciais** para a corretude.

### J19. (Aproximação) Variante de Christofides com dois emparelhamentos perfeitos de peso mínimo (cuja união tem todo vértice de grau 2) é ótima?
**Não.** A **união dos dois emparelhamentos não precisa ser conexa**, então o grafo resultante
**não é necessariamente Euleriano** (Euleriano exige conexidade além de graus pares). O argumento
do colega falha nesse ponto.

### J20. (Dijkstra) Para caminho s-t, dois critérios de parada agressivos: (a) parar ao processar v com d_v = d_t; (b) parar quando t é atualizado (decreasekey em t). Corretos?
(a) **Correto.** A distância de `t` só pode diminuir, mas Dijkstra processa em **ordem crescente
de distância**; quando processa um vértice com `d_v = d_t`, a distância de `t` já é a mínima.
(b) **Errado.** Contra-exemplo: triângulo `s-v-t` com pesos `sv=1`, `vt=1`, `st=3`. `t` é
atualizado cedo (via `st=3`) mas o caminho ótimo `s-v-t` (custo 2) ainda não foi descoberto.

### J21. (Caminhos disjuntos) Algoritmo eficiente para o número máximo de caminhos arco-disjuntos entre s e t.
**Redução a fluxo máximo** com capacidades **unitárias** em todos os arcos. Resolvendo com um
algoritmo do tipo Ford-Fulkerson, o fluxo é **integral** e pode ser **decomposto** em caminhos
arco-disjuntos; o valor do fluxo é igual ao número máximo de caminhos arco-disjuntos.

### J22. (Emparelhamentos) Algoritmo randomizado que escolhe arestas livres aleatoriamente é uma 2-aproximação para o emparelhamento máximo?
**Sim** (qualquer emparelhamento **maximal** é 2-aproximação). Suponha que o emparelhamento
máximo tem cardinalidade `k` e o encontrado tem `< ⌊k/2⌋`. Dos `2k` vértices cobertos pelo
máximo, pelo menos `k+1` ficariam livres; pelo **princípio da casa dos pombos**, existiria pelo
menos uma aresta do emparelhamento máximo com **os dois vértices livres**, que poderia ser
adicionada, contradizendo a maximalidade.

### J23. (Heaps) Compare heap binário com heap oco (operações e complexidade).
- **Heap binário:** `insert` `O(log n)`, `deletemin` `O(log n)`, `decreasekey` `O(log n)`.
- **Heap oco (hollow heap):** `insert` `O(1)`, `deletemin` `O(log n)`, `decreasekey` `O(1)`
  **amortizado**.
O heap oco melhora `insert` e `decreasekey` para `O(1)` amortizado, vantajoso em algoritmos com
muitos `decreasekey` (como Dijkstra/Prim em grafos densos).
