#include   <stdio.h>
#include   <string.h>
#include   <memory.h>
#include   <malloc.h>
#include   <assert.h>

#define LISTA_OWN
#include "Lista.h"
#undef LISTA_OWN

/***********************************************************************
*
*  $TC Tipo de dados: LIS Elemento da lista
*
*
***********************************************************************/

typedef struct tagElemLista {

	void * pValor;
	/* Ponteiro para o valor contido no elemento */

	struct tagElemLista * pAnt;
	/* Ponteiro para o elemento predecessor */

	struct tagElemLista * pProx;
	/* Ponteiro para o elemento sucessor */

} tpElemLista;

/***********************************************************************
*
*  $TC Tipo de dados: LIS Descritor da cabe�a de lista
*
*
***********************************************************************/

typedef struct LIS_tagLista {

	tpElemLista * pOrigemLista;
	/* Ponteiro para a origem da lista */

	tpElemLista * pFimLista;
	/* Ponteiro para o final da lista */

	tpElemLista * pElemCorr;
	/* Ponteiro para o elemento corrente da lista */

	int numElem;
	/* N�mero de elementos da lista */

	void(*ExcluirValor) (void * pValor);
	/* Ponteiro para a fun��o de destrui��o do valor contido em um elemento */

} LIS_tpLista;

/***** Prot�tipos das fun��es encapuladas no m�dulo *****/

static void LiberarElemento(LIS_tppLista   pLista,
	tpElemLista  * pElem);

static tpElemLista * CriarElemento(LIS_tppLista pLista,
	void *       pValor);

static void LimparCabeca(LIS_tppLista pLista);


/**** Variaveis est�ticas de controle *****/

static tpElemLista * posInicialCorr;


/*****  C�digo das fun��es exportadas pelo m�dulo  *****/

/***************************************************************************
*
*  Fun��o: LIS  &Criar lista
*  ****/

LIS_tppLista LIS_CriarLista(
	void(*ExcluirValor) (void * pDado))
{

	LIS_tpLista * pLista = NULL;

	pLista = (LIS_tpLista *)malloc(sizeof(LIS_tpLista));
	if (pLista == NULL)
	{
		return NULL;
	} /* if */

	LimparCabeca(pLista);

	pLista->ExcluirValor = ExcluirValor;

	return pLista;

} /* Fim fun��o: LIS  &Criar lista */

/***************************************************************************
*
*  Fun��o: LIS  &Tamanho lista
*  ****/
int LIS_TamanhoLista(LIS_tppLista pLista){
    return pLista->numElem;
}


/***************************************************************************
*
*  Fun��o: LIS  &Destruir lista
*  ****/

void LIS_DestruirLista(LIS_tppLista pLista)
{

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	LIS_EsvaziarLista(pLista);

	free(pLista);

} /* Fim fun��o: LIS  &Destruir lista */

/***************************************************************************
*
*  Fun��o: LIS  &Esvaziar lista
*  ****/

void LIS_EsvaziarLista(LIS_tppLista pLista)
{

	tpElemLista * pElem;
	tpElemLista * pProx;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	pElem = pLista->pOrigemLista;
	while (pElem != NULL)
	{
		pProx = pElem->pProx;
		LiberarElemento(pLista, pElem);
		pElem = pProx;
	} /* while */

	LimparCabeca(pLista);

} /* Fim fun��o: LIS  &Esvaziar lista */

/***************************************************************************
*
*  Fun��o: LIS  &Inserir elemento antes
*  ****/

LIS_tpCondRet LIS_InserirElementoAntes(LIS_tppLista pLista,
	void * pValor)
{

	tpElemLista * pElem;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	/* Criar elemento a inerir antes */

	pElem = CriarElemento(pLista, pValor);
	if (pElem == NULL)
	{
		return LIS_CondRetFaltouMemoria;
	} /* if */

 /* Encadear o elemento antes do elemento corrente */

	if (pLista->pElemCorr == NULL)
	{
		pLista->pOrigemLista = pElem;
		pLista->pFimLista = pElem;
	}
	else
	{
		if (pLista->pElemCorr->pAnt != NULL)
		{
			pElem->pAnt = pLista->pElemCorr->pAnt;
			pLista->pElemCorr->pAnt->pProx = pElem;
		}
		else
		{
			pLista->pOrigemLista = pElem;
		} /* if */

		pElem->pProx = pLista->pElemCorr;
		pLista->pElemCorr->pAnt = pElem;
	} /* if */

	pLista->pElemCorr = pElem;
	return LIS_CondRetOK;

} /* Fim fun��o: LIS  &Inserir elemento antes */

/***************************************************************************
*
*  Fun��o: LIS  &Inserir elemento ap�s
*  ****/

LIS_tpCondRet LIS_InserirElementoApos(LIS_tppLista pLista,
	void * pValor)

{

	tpElemLista * pElem;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	/* Criar elemento a inerir ap�s */

	pElem = CriarElemento(pLista, pValor);
	if (pElem == NULL)
	{
		return LIS_CondRetFaltouMemoria;
	} /* if */

 /* Encadear o elemento ap�s o elemento */

	if (pLista->pElemCorr == NULL)
	{
		pLista->pOrigemLista = pElem;
		pLista->pFimLista = pElem;
	}
	else
	{
		if (pLista->pElemCorr->pProx != NULL)
		{
			pElem->pProx = pLista->pElemCorr->pProx;
			pLista->pElemCorr->pProx->pAnt = pElem;
		}
		else
		{
			pLista->pFimLista = pElem;
		} /* if */

		pElem->pAnt = pLista->pElemCorr;
		pLista->pElemCorr->pProx = pElem;

	} /* if */

	pLista->pElemCorr = pElem;
	return LIS_CondRetOK;

} /* Fim fun��o: LIS  &Inserir elemento ap�s */

/***************************************************************************
*
*  Fun��o: LIS  &Inserir elemento no fim
*  ****/

   LIS_tpCondRet LIS_InserirElementoFim( LIS_tppLista pLista ,
                                          void * pValor        )
      
   {

      tpElemLista * pElem ;
      tpElemLista * final;

      #ifdef _DEBUG
         assert( pLista != NULL ) ;
      #endif

      /* Criar elemento a inerir ap�s */

         pElem = CriarElemento( pLista , pValor ) ;
         if ( pElem == NULL )
         {
            return LIS_CondRetFaltouMemoria ;
         } /* if */

      /* Encadear o elemento ap�s o elemento */

         if ( pLista->pElemCorr == NULL )
         {
            pLista->pOrigemLista = pElem ;
            pLista->pFimLista = pElem ;
         	pLista->pElemCorr = pElem;
         	return LIS_CondRetOK;
         } 
         else
         {
            final = pLista->pFimLista;
            pLista->pFimLista->pProx = pElem;
            pElem->pAnt = final;
            pElem->pProx = NULL;
            
           	pLista->pFimLista = pElem;
            return LIS_CondRetOK;
         } /* if */
                  
                  

   } /* Fim fun��o: LIS  &Inserir elemento fim */

/***************************************************************************
*
*  Fun��o: LIS  &Inserir elemento ap�s
*  ****/

LIS_tpCondRet LIS_InserirElementoFim2(LIS_tppLista pLista,
	void * pValor)

{

	tpElemLista * pElem;
	tpElemLista * pFim;
#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	/* Criar elemento a inerir ap�s */

	pElem = CriarElemento(pLista, pValor);
	if (pElem == NULL)
	{
		return LIS_CondRetFaltouMemoria;
	} /* if */
	pFim = pLista->pFimLista;
	if (pFim == NULL) {
		pLista->pFimLista = pElem;
		pLista->pOrigemLista = pElem;
	}
	else
	{
		pFim->pProx = pElem;
		pElem->pAnt = pFim;
	}
	return LIS_CondRetOK;
} /* Fim fun��o: LIS  &Inserir elemento ap�s */



/***************************************************************************
*
*  Fun��o: LIS  &Excluir elemento
*  ****/

LIS_tpCondRet LIS_ExcluirElemento(LIS_tppLista pLista)
{

	tpElemLista * pElem;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	if (pLista->pElemCorr == NULL)
	{
		return LIS_CondRetListaVazia;
	} /* if */

	pElem = pLista->pElemCorr;

	/* Desencadeia � esquerda */

	if (pElem->pAnt != NULL)
	{
		pElem->pAnt->pProx = pElem->pProx;
		pLista->pElemCorr = pElem->pAnt;
	}
	else {
		pLista->pElemCorr = pElem->pProx;
		pLista->pOrigemLista = pLista->pElemCorr;
	} /* if */

 /* Desencadeia � direita */

	if (pElem->pProx != NULL)
	{
		pElem->pProx->pAnt = pElem->pAnt;
	}
	else
	{
		pLista->pFimLista = pElem->pAnt;
	} /* if */

	LiberarElemento(pLista, pElem);
	return LIS_CondRetOK;

} /* Fim fun��o: LIS  &Excluir elemento */

/***************************************************************************
*
*  Fun��o: LIS  &Excluir elemento
*	
*	$ED: Retorna o valor do N� a ser excluido.
*  ****/

LIS_tpCondRet LIS_ExcluirElementoOutput(LIS_tppLista pLista, void ** output)
{

	tpElemLista * pElem;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	if (pLista->pElemCorr == NULL)
	{
		return LIS_CondRetListaVazia;
	} /* if */

	pElem = pLista->pElemCorr;

	/* Desencadeia � esquerda */

	if (pElem->pAnt != NULL)
	{
		pElem->pAnt->pProx = pElem->pProx;
		pLista->pElemCorr = pElem->pAnt;
	}
	else {
		pLista->pElemCorr = pElem->pProx;
		pLista->pOrigemLista = pLista->pElemCorr;
	} /* if */

 /* Desencadeia � direita */

	if (pElem->pProx != NULL)
	{
		pElem->pProx->pAnt = pElem->pAnt;
	}
	else
	{
		pLista->pFimLista = pElem->pAnt;
	} /* if */

	
	LiberarElementoOutput(pLista, pElem,output);
	return LIS_CondRetOK;

} /* Fim fun��o: LIS  &Excluir elemento */




/***************************************************************************
*
*  Fun��o: LIS  &Obter refer�ncia para o valor contido no elemento
*  ****/

void * LIS_ObterValor(LIS_tppLista pLista)
{

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	if (pLista->pElemCorr == NULL)
	{
		return NULL;
	} /* if */

	return pLista->pElemCorr->pValor;

} /* Fim fun��o: LIS  &Obter refer�ncia para o valor contido no elemento */

void SalvaCorrente(LIS_tppLista pLista){
	posInicialCorr = pLista->pElemCorr;
}

void ResetaCorrente(LIS_tppLista pLista){
	pLista->pElemCorr = posInicialCorr;
	posInicialCorr = NULL;
}




/***************************************************************************
*
*  Fun��o: LIS  &Ir para o elemento inicial
*  ****/

void IrInicioLista(LIS_tppLista pLista)
{

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	pLista->pElemCorr = pLista->pOrigemLista;

} /* Fim fun��o: LIS  &Ir para o elemento inicial */

/***************************************************************************
*
*  Fun��o: LIS  &Ir para o elemento final
*  ****/

void IrFinalLista(LIS_tppLista pLista)
{

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	pLista->pElemCorr = pLista->pFimLista;

} /* Fim fun��o: LIS  &Ir para o elemento final */


/***************************************************************************
*
*  Fun��o: LIS  &Avan�ar elemento
*  ****/

   LIS_tpCondRet LIS_AvancarElementoCorrente( LIS_tppLista pLista ,
                                              int numElem )
   {

      int i ;

      tpElemLista * pElem ;

      #ifdef _DEBUG
         assert( pLista != NULL ) ;
      #endif

      /* Tratar lista vazia */

         if ( pLista->pElemCorr == NULL )
         {

            return LIS_CondRetListaVazia ;

         } /* fim ativa: Tratar lista vazia */

      /* Tratar avan�ar para frente */

         if ( numElem > 0 )
         {

            pElem = pLista->pElemCorr ;
            for( i = numElem ; i > 0 ; i-- )
            {
               if ( pElem == NULL )
               {
                  pElem = pLista->pOrigemLista;
               } /* if */
               else{
                  pElem = pElem->pProx ;
               }
            } /* for */

            if ( pElem != NULL )
            {
               pLista->pElemCorr = pElem ;
               return LIS_CondRetOK ;
            } /* if */
			
            pLista->pElemCorr = pLista->pOrigemLista;
            return LIS_CondRetFimLista ;

         } /* fim ativa: Tratar avan�ar para frente */

      /* Tratar avan�ar para tr�s */

         else if ( numElem < 0 )
         {

            pElem = pLista->pElemCorr ;
            for( i = numElem ; i < 0 ; i++ )
            {
               if ( pElem == NULL )
               {
                  pElem = pLista->pFimLista;
               } /* if */
               pElem = pElem->pAnt ;
            } /* for */

            if ( pElem != NULL )
            {
               pLista->pElemCorr = pElem ;
               return LIS_CondRetOK ;
            } /* if */
			pLista->pElemCorr = pLista->pFimLista;
            return LIS_CondRetFimLista ;

         } /* fim ativa: Tratar avan�ar para tr�s */

      /* Tratar n�o avan�ar */

         return LIS_CondRetOK ;

   } /* Fim fun��o: LIS  &Avan�ar elemento */


/***************************************************************************
*
*  Fun��o: LIS  &Procurar elemento contendo valor
*  ****/

LIS_tpCondRet LIS_ProcurarValor(LIS_tppLista pLista,
	void * pValor)
{

	tpElemLista * pElem;

#ifdef _DEBUG
	assert(pLista != NULL);
#endif

	if (pLista->pElemCorr == NULL)
	{
		return LIS_CondRetListaVazia;
	} /* if */

	for (pElem = pLista->pElemCorr;
		pElem != NULL;
		pElem = pElem->pProx)
	{
		if (pElem->pValor == pValor)
		{
			pLista->pElemCorr = pElem;
			return LIS_CondRetOK;
		} /* if */
	} /* for */

	return LIS_CondRetNaoAchou;

} /* Fim fun��o: LIS  &Procurar elemento contendo valor */


/*****  C�digo das fun��es encapsuladas no m�dulo  *****/


/***********************************************************************
*
*  $FC Fun��o: LIS  -Liberar elemento da lista
*
*  $ED Descri��o da fun��o
*     Elimina os espa�os apontados pelo valor do elemento e o
*     pr�prio elemento.
*
***********************************************************************/

void LiberarElemento(LIS_tppLista   pLista,
	tpElemLista  * pElem)
{

	if ((pLista->ExcluirValor != NULL)
		&& (pElem->pValor != NULL))
	{
		pLista->ExcluirValor(pElem->pValor);
	} /* if */

	free(pElem);
	pLista->numElem--;

} /* Fim fun��o: LIS  -Liberar elemento da lista */

/***********************************************************************
*
*  $FC Fun��o: LIS  -Liberar elemento da lista
*
*  $ED Descri��o da fun��o
*     Elimina o pr�prio elemento e devolve o valor por ele apontado.
*
***********************************************************************/

void LiberarElementoOutput(LIS_tppLista   pLista,
	tpElemLista  * pElem, void ** valor)
{
	if (pElem->pValor != NULL)
	{
		*valor = pElem->pValor;//void * pValr
		//pLista->ExcluirValor(pElem->pValor);
	} /* if */

	free(pElem);
	pLista->numElem--;
} /* Fim fun��o: LIS  -Liberar elemento da lista */



/***********************************************************************
*
*  $FC Fun��o: LIS  -Criar o elemento
*
***********************************************************************/

tpElemLista * CriarElemento(LIS_tppLista pLista,
	void *       pValor)
{

	tpElemLista * pElem;

	pElem = (tpElemLista *)malloc(sizeof(tpElemLista));
	if (pElem == NULL)
	{
		return NULL;
	} /* if */

	pElem->pValor = pValor;
	pElem->pAnt = NULL;
	pElem->pProx = NULL;

	pLista->numElem++;

	return pElem;

} /* Fim fun��o: LIS  -Criar o elemento */


/***********************************************************************
*
*  $FC Fun��o: LIS  -Limpar a cabe�a da lista
*
***********************************************************************/

void LimparCabeca(LIS_tppLista pLista)
{

	pLista->pOrigemLista = NULL;
	pLista->pFimLista = NULL;
	pLista->pElemCorr = NULL;
	pLista->numElem = 0;

} /* Fim fun��o: LIS  -Limpar a cabe�a da lista */


/********** Fim do m�dulo de implementa��o: LIS  Lista duplamente encadeada **********/



