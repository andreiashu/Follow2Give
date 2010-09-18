<?php

twitter_integration_include();

function twitter_goto_twitter_login($return_url = NULL) {
  $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET);

  if($return_url) {
    drupal_goto($twitterObj->getAuthorizeUrl(null,array('oauth_callback' => WEBSITE_URL . '/' . $return_url)));
  }
  else {
    drupal_goto($twitterObj->getAuthorizeUrl());
  }
}

function twitter_return_load_user() {
  $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET);

  try {
    $twitterObj->setToken($_GET['oauth_token']);
    $token = $twitterObj->getAccessToken(array('oauth_verifier' => $_GET['oauth_verifier']));
    $account = user_external_load($token->user_id);

    if (!isset($account->uid)) { // current user
      $account = twitter_user_save($response, $token);
    }

    $account->twitter_token = $token;

  } catch (EpiException $e) {
    drupal_set_message("Twitter has an error - please try again: " . $e->getMessage());
    return FALSE;
  }

  return $account;
}

function twitter_user_save($token) {
  if($account = user_load(array('name' => $token->screen_name))) {
    return $account;
  }

  $edit = array(
    'name' => $token->screen_name,
    'pass' => user_password(),
    'init' => $token->screen_name,
    'status' => 1,
    "authname_twitter" => $token->user_id,
    'access' => time(),
  );
  $account = user_save('', $edit);

  return $account;
}

function twitter_integration_include() {
  static $called = FALSE;
  if($called) {
    return;
  }
  $called = TRUE;
  $path = drupal_get_path('module', 'twitter_integration') . '/includes/twitter-async';

  require_once($path . '/' . 'EpiCurl.php');
  require_once($path . '/' . 'EpiOAuth.php');
  require_once($path . '/' . 'EpiTwitter.php');
}

function twitter_oauth_user_load($uid) {
  return db_fetch_object(db_query("SELECT * FROM {f2g_twitter_tokens} WHERE uid = %d", $uid));
}