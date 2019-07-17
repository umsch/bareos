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

namespace Api\Model;

class ApiModel
{
    /**
     * @param $
     */
    public function executeCommand(&$bsock, $command)
    {
           throw new \Exception('Not implemented.');
    }

   /**
    * Get mulitple Apis
    *
    * @param $bsock
    * @param $apiname
    * @param $days
    *
    * @return array
    */
   public function getApis(&$bsock=null, $apiname=null, $days=null)
   {
      if (isset($bsock)) {
         if ($days == "all") {
            if ($apiname == "all") {
               $cmd = 'llist apis';
            } else {
               $cmd = 'llist apis apiname="'.$apiname.'"';
            }
         } else  {
            if ($apiname == "all") {
               $cmd = 'llist apis days='.$days;
            } else {
               $cmd = 'llist apis apiname="'.$apiname.'" days='.$days;
            }
         }
         $limit = 1000;
         $offset = 0;
         $retval = array();
         while (true) {
            $result = $bsock->send_command($cmd . ' limit=' . $limit . ' offset=' . $offset, 2, null);
            if (preg_match('/Failed to send result as json. Maybe result message to long?/', $result)) {
               $error = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               return $error['result']['error'];
            } else {
               $apis = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               if ( empty($apis['result']) ) {
                  return false; // No matching records found
               }
               if ( empty($apis['result']['apis']) && $apis['result']['meta']['range']['filtered'] === 0 ) {
                  return $retval;
               } else {
                  $retval = array_merge($retval, $apis['result']['apis']);
               }
            }
            $offset = $offset + $limit;
         }
      } else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get Api by Status
    *
    * @param $bsock
    * @param $apiname
    * @param $status
    * @param $days
    * @param $hours
    *
    * @return array
    */
   public function getApisByStatus(&$bsock=null, $apiname=null, $status=null, $days=null, $hours=null)
   {
      if (isset($bsock, $status)) {
         if (isset($days)) {
            if ($days == "all") {
               $cmd = 'llist apis apistatus='.$status.'';
            } else {
               $cmd = 'llist apis apistatus='.$status.' days='.$days.'';
            }
         } elseif (isset($hours)) {
            if ($hours == "all") {
               $cmd = 'llist apis apistatus='.$status.'';
            } else {
               $cmd = 'llist apis apistatus='.$status.' hours='.$hours.'';
            }
         } else {
            $cmd = 'llist apis apistatus='.$status.'';
         }
         if ($apiname != "all") {
            $cmd .= ' apiname="'.$apiname.'"';
         }
         $limit = 1000;
         $offset = 0;
         $retval = array();
         while (true) {
            $result = $bsock->send_command($cmd . ' limit=' . $limit . ' offset=' . $offset, 2, null);
            if (preg_match('/Failed to send result as json. Maybe result message to long?/', $result)) {
               $error = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               return $error['result']['error'];
            } else {
               $apis = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               if(empty($result)) {
                  return false;
               }
               if ( empty($apis['result']['apis']) && $apis['result']['meta']['range']['filtered'] === 0 ) {
                  return array_reverse($retval);
               } else {
                  $retval = array_merge($retval, $apis['result']['apis']);
               }
            }
            $offset = $offset + $limit;
         }
      } else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get a single Api
    *
    * @param $bsock
    * @param $id
    *
    * @return array
    */
   public function getApi(&$bsock=null, $id=null)
   {
      if(isset($bsock, $id)) {
         $cmd = 'llist apiid='.$id.'';
         $result = $bsock->send_command($cmd, 2, null);
         $api = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         if ( empty($api['result']) ) {
            return false; // No matching records found
         } else {
            return $api['result']['apis'];
         }
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get Api Log
    *
    * @param $bsock
    * @param $id
    *
    * @return array
    */
   public function getApiLog(&$bsock=null, $id=null)
   {
      if(isset($bsock, $id)) {
         $cmd = 'list apilog apiid='.$id.'';
         $limit = 1000;
         $offset = 0;
         $retval = array();
         while (true) {
            $result = $bsock->send_command($cmd . ' limit=' . $limit . ' offset=' . $offset, 2, null);
            if(preg_match('/Failed to send result as json. Maybe result message to long?/', $result)) {
               $error = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               return $error['result']['error'];
            }
            else {
               $log = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               if ( empty($log['result']) ) {
                  return false; // No matching records found
               }
               if ( empty($log['result']['apilog']) && $log['result']['meta']['range']['filtered'] === 0 ) {
                  return $retval;
               } else {
                  $retval = array_merge($retval, $log['result']['apilog']);
               }
            }
            $offset = $offset + $limit;
         }
      } else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get Api Media
    *
    * @param $bsock
    * @param $apiid
    *
    * @return array
    */
   public function getApiMedia(&$bsock=null, $apiid=null)
   {
      $cmd = 'llist apimedia apiid='.$apiid;
      $result = $bsock->send_command($cmd, 2, null);
      if(preg_match('/Failed to send result as json. Maybe result message to long?/', $result)) {
         //return false;
         $error = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $error['result']['error'];
      }
      else {
         $apimedia = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $apimedia['result']['apimedia'];
      }
   }

   /**
    * Get Apis by type
    *
    * @param $bsock
    * @param $type
    *
    * @return array
    */
   public function getApisByType(&$bsock=null, $type=null)
   {
      if(isset($bsock)) {
         if($type == null) {
            $cmd = '.apis';
         }
         else {
            $cmd = '.apis type="'.$type.'"';
         }
         $result = $bsock->send_command($cmd, 2, null);
         $apis = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $apis['result']['apis'];
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get ApisLastStatus
    *
    * @param $bsock
    *
    * @return array
    */
   public function getApisLastStatus(&$bsock=null)
   {
      if(isset($bsock)) {
         $cmd = 'llist apis last current enabled';
         $result = $bsock->send_command($cmd, 2, null);
         $apis = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $apis['result']['apis'];
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get ApiTotals
    *
    * @param $bsock
    *
    * @return array
    */
   public function getApiTotals(&$bsock=null)
   {
      if(isset($bsock)) {
         $cmd = 'list apitotals';
         $result = $bsock->send_command($cmd, 2, null);
         $apitotals = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return array(0 => $apitotals['result']['apitotals']);
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get Running Apis Statistics
    *
    * @param $bsock
    *
    * @return array
    */
   public function getRunningApisStatistics(&$bsock = null) {
      if(isset($bsock)) {

         $apistats = array();
         $i = 0;

         // GET RUNNING JOBS
         $runningApis = $this->getApisByStatus($bsock, null, 'R');

         // COLLECT REQUIRED DATA FOR EACH RUNNING JOB
         foreach($runningApis as $api) {

            // GET THE JOB STATS
            $cmd = 'list apistatistics apiid=' . $api['apiid'];
            $result = $bsock->send_command($cmd, 2, null);
            $tmp = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);

            // JOBID, JOBNAME AND CLIENT
            $apistats[$i]['apiid'] = $api['apiid'];
            $apistats[$i]['name'] = $api['name'];
            $apistats[$i]['client'] = $api['client'];
            $apistats[$i]['level'] = $api['level'];

            if(count($tmp['result']['apistats']) > 2) {

               // CALCULATE THE CURRENT TRANSFER SPEED OF THE INTERVAL
               $a = strtotime( $tmp['result']['apistats'][count($tmp['result']['apistats']) - 1]['sampletime'] );
               $b = strtotime( $tmp['result']['apistats'][count($tmp['result']['apistats']) - 2]['sampletime'] );
               $interval = $a - $b;

               if($interval > 0) {
                  $speed = ($tmp['result']['apistats'][count($tmp['result']['apistats']) - 1]['apibytes'] - $tmp['result']['apistats'][count($tmp['result']['apistats']) - 2]['apibytes']) / $interval;
                  $speed = round($speed, 2);
               }
               else {
                  $speed = 0;
               }

               $apistats[$i]['speed'] = $speed;

               // JOBFILES
               $tmp = $tmp['result']['apistats'][count($tmp['result']['apistats']) - 1];
               if($tmp['apifiles'] == null) {
                  $apistats[$i]['apifiles'] = 0;
               }
               else {
                  $apistats[$i]['apifiles'] = $tmp['apifiles'];
               }

               // JOBBYTES
               $apistats[$i]['apibytes'] = $tmp['apibytes'];

               // SAMPLETIME
               $apistats[$i]['sampletime'] = $tmp['sampletime'];

               // LAST BACKUP SIZE
               $level = $apistats[$i]['level'];
               $cmd = 'list apis apiname=' . $api['name'] . ' client=' . $api['client'] . ' apistatus=T apilevel=' . $level . ' last';

               $result = $bsock->send_command($cmd, 2, null);
               $tmp = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
               $apistats[$i]['lastbackupsize'] = $tmp['result']['apis'][0]['apibytes'];
               if($apistats[$i]['lastbackupsize'] > 0) {
                  if($apistats[$i]['apibytes'] > 0 && $tmp['result']['apis'][0]['apibytes'] > 0) {
                     $apistats[$i]['progress'] = ceil( (($apistats[$i]['apibytes'] * 100) / $tmp['result']['apis'][0]['apibytes']));
                     if($apistats[$i]['progress'] > 100) {
                        $apistats[$i]['progress'] = 99;
                     }
                  }
                  else {
                     $apistats[$i]['progress'] = 0;
                  }
               }
               else {
                  $apistats[$i]['progress'] = 0;
               }
            }
            else {
               $apistats[$i]['speed'] = 0;
               $apistats[$i]['apifiles'] = 0;
               $apistats[$i]['apibytes'] = 0;
               $apistats[$i]['sampletime'] = null;
               $apistats[$i]['progress'] = 0;
            }

            $i++;

         }
         return $apistats;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get the available Restore Apis
    *
    * @param $bsock
    *
    * @return array
    */
   public function getRestoreApis(&$bsock=null)
   {
      if(isset($bsock)) {
         $cmd = '.apis type=R';
         $result = $bsock->send_command($cmd, 2, null);
         $restoreapis = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $restoreapis['result']['apis'];
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Run a api as scheduled
    *
    * @param $bsock
    * @param $name
    *
    * @return string
    */
   public function runApi(&$bsock=null, $name=null)
   {
      if(isset($bsock, $name)) {
         $cmd = 'run api="'.$name.'" yes';
         $result = $bsock->send_command($cmd, 0, null);
         return $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Get api defaults
    *
    * @param $bsock
    * @param $name
    *
    * @return array
    */
   public function getApiDefaults(&$bsock=null, $name=null)
   {
      if(isset($bsock, $name)) {
         $cmd = '.defaults api="'.$name.'"';
         $result = $bsock->send_command($cmd, 2, null);
         $apidefaults = \Zend\Json\Json::decode($result, \Zend\Json\Json::TYPE_ARRAY);
         return $apidefaults['result']['defaults'];
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Run a custom api
    *
    * @param $bsock
    * @param $apiname
    * @param $client
    * @param $fileset
    * @param $storage
    * @param $pool
    * @param $level
    * @param $priority
    * @param $backupformat
    * @param $when
    *
    * @return string
    */
   public function runCustomApi(&$bsock=null, $apiname=null, $client=null, $fileset=null, $storage=null, $pool=null, $level=null, $priority=null, $backupformat=null, $when=null)
   {
      if(isset($bsock, $apiname)) {
         $cmd = 'run api="' . $apiname . '"';
         if(!empty($client)) {
            $cmd .= ' client="' . $client . '"';
         }
         if(!empty($fileset)) {
            $cmd .= ' fileset="' . $fileset . '"';
         }
         if(!empty($storage)) {
            $cmd .= ' storage="' . $storage . '"';
         }
         if(!empty($pool)) {
            $cmd .= ' pool="' . $pool . '"';
         }
         if(!empty($level)) {
            $cmd .= ' level="' . $level . '"';
         }
         if(!empty($priority)) {
            $cmd .= ' priority="' . $priority . '"';
         }
         if(!empty($backupformat)) {
            $cmd .= ' backupformat="' . $backupformat . '"';
         }
         if(!empty($when)) {
            $cmd .= ' when="' . $when . '"';
         }
         $cmd .= ' yes';
         $result = $bsock->send_command($cmd, 0 , null);
         return 'Command send: '. $cmd . ' | Director message: ' . $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Re-Run a api
    *
    * @param $bsock
    * @param $id
    *
    * @return string
    */
   public function rerunApi(&$bsock=null, $id=null)
   {
      if(isset($bsock, $id)) {
         $cmd = 'rerun apiid='.$id.' yes';
         $result = $bsock->send_command($cmd, 0, null);
         return $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Cancel a api
    *
    * @param $bsock
    * @param $id
    *
    * @return string
    */
   public function cancelApi(&$bsock=null, $id=null)
   {
      if(isset($bsock, $id)) {
         $cmd = 'cancel apiid='.$id.' yes';
         $result = $bsock->send_command($cmd, 0, null);
         return $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Enable a api
    *
    * @param $bsock
    * @param $name
    *
    * @return string
    */
   public function enableApi(&$bsock=null, $name=null)
   {
      if(isset($bsock, $name)) {
         $cmd = 'enable api="'.$name.'" yes';
         $result = $bsock->send_command($cmd, 0, null);
         return $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }

   /**
    * Disable a api
    *
    * @param $bsock
    * @param $name
    *
    * @return string
    */
   public function disableApi(&$bsock=null, $name=null)
   {
      if(isset($bsock, $name)) {
         $cmd = 'disable api="'.$name.'" yes';
         $result = $bsock->send_command($cmd, 0, null);
         return $result;
      }
      else {
         throw new \Exception('Missing argument.');
      }
   }
}
