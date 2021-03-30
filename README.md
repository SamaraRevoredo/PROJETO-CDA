# Projeto SDR
Pasta raíz de projeto.



## 1. Fazer o download do [Git](https://git-scm.com/download/win) e para como instalar link [Youtube](https://www.youtube.com/watch?v=SOxafinthys&ab_channel=ProfessorJos%C3%A9deAssis)


## 2. Clonar o repositório: https://github.com/SD-G2-S9/PROJETO-SDR.git na pasta preferida apertando com botão direito do mouse e escolhendo a opção **Git Bash Here**.

Copie o seguinte comando e cole no terminal: 
> git clone https://github.com/SD-G2-S9/PROJETO-SDR.git



## 3. Primeiros passos do projeto:


3.1 - Com o terminal para saber qual branch você está digite:
>git branch

3.2 - Caso esteja na **master**, vá para a branch **develop**, então digite:
>git checkout develop

3.4 - Se estiver desenvolvendo um recurso novo como código separado do display LCD, funcionando da leitura AD e outros. Então crie uma branch feature:
>git checkout -b feature_nomeBranch

Continue seu trabalho e use o Git como de costume.


3.5 - Quando você concluir o trabalho de desenvolvimento no recurso, a próxima etapa é mesclar a ramificação de recurso na de desenvolvimento:

>git checkout develop
>git merge feature_nomeBranch



