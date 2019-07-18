<?php

/**
 *
 * bareos-webui - Bareos Web-Frontend
 *
 * @link      https://github.com/bareos/bareos-webui for the canonical source repository
 * @copyright Copyright (c) 2013-2017 Bareos GmbH & Co. KG (http://www.bareos.org/)
 * @license   GNU Affero General Public License (http://www.gnu.org/licenses/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Api\Controller;

use Zend\Mvc\Controller\AbstractActionController;
use Zend\Json\Json;
use Zend\Http\Response;

class ApiController extends AbstractActionController
{

    protected $apiModel = null;
    protected $bsock = null;

    public function indexAction()
    {
        $this->RequestURIPlugin()->setRequestURI();

        if (!$this->SessionTimeoutPlugin()->isValid()) {
            return $this->redirect()->toRoute(
                'auth', array('action' => 'login'),
                array('query' => array('req'  => $this->RequestURIPlugin()
                    ->getRequestURI(),
                                       'dird' => $_SESSION['bareos']['director']))
            );
        }

        $response = $this->getResponse();
        $request = $this->getRequest();

        if (!$request->isPost()) {
            $response->setStatusCode(Response::STATUS_CODE_404);
        } else {
            $this->bsock = $this->getServiceLocator()->get('director');

            $raw = $this->request->getContent();
            $request = Json::decode($raw);

            // und ab in die bconsole
            $model = $this->getApiModel();
            $cmdResult = $model->executeCommand($this->bsock, $request->cmd);

            $request->result = $cmdResult;

            $response = $this->getResponse();
            $response->getHeaders()->addHeaderLine(
                'Content-Type', 'application/json'
            );

            $response->setContent(Json::encode($request));
        }

        return $response;
    }

    public function getApiModel()
    {
        if (!$this->apiModel) {
            $sm = $this->getServiceLocator();
            $this->apiModel = $sm->get('Api\Model\ApiModel');
        }
        return $this->apiModel;
    }

}